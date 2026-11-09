#!/usr/bin/env python3
"""
build_and_log.py — Simple parallel build runner with shared error logging.

This is the baseline script for a simple parallel build: it builds each
config example and logs any failures to a shared log file so problems can
be triaged after all parallel builds have completed.

Usage:
    python3 build_and_log.py [--loop-limit=N] [--resume=DIR] [--limit=N]
                              [--nofail] [--many] [--archive] [-a]

Distributed builds (--hosts):
    python3 parallel-build_and_log.py --hosts=local,thinkyhead@sharky.local:4,\
        thinkycore@minicore.local --output=~/MarlinBuilds/2.1.3 --archive --nofail

    Each host gets a pinned checkout of Marlin and Configurations in its own
    work folder (default ~/MarlinBuilds-work), builds its share of the example
    configs (build_all_examples --shard), and the archives and error logs are
    collected here, with failures tagged by host. A host spec is
    [user@]host[:weight]; the weight defaults to the host's core count.
    Use --plan to see the split without touching any host.

The log file is written to .pio/build_errors.log inside MARLIN_REPO,
so all parallel clones write to the same shared location when their
MARLIN_REPO env var points at the same place (or can be merged later).
"""

import subprocess
import sys
import os
import re
import argparse
import signal
import datetime
import shlex
import threading
import time
import shutil

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PLATFORMIO_SCRIPTS = os.path.join(SCRIPT_DIR, '..', 'share', 'PlatformIO', 'scripts')
sys.path.insert(0, PLATFORMIO_SCRIPTS)
import config  # type: ignore[import-untyped]

# ---------------------------------------------------------------------------
# Environment — derive repo path from CWD (not a hardcoded env var)
# ---------------------------------------------------------------------------
MARLIN_REPO  = os.getcwd()
MARLIN_CONFIGS = os.environ.get('MARLIN_CONFIGS', '')
BUILDALL     = os.path.join(MARLIN_REPO, 'buildroot', 'bin', 'build_all_examples')
STAT_FILE    = os.path.join(MARLIN_REPO, '.pio', '.buildall')
LOG_FILE     = os.path.join(MARLIN_REPO, '.pio', 'build_errors.log')

# How many lines to capture from the tail of failed build output
TAIL_LINES = 40


def run_build(build_opts=None, resume=None):
    """Run build_all_examples. Returns (returncode, combined_output)."""
    cmd = [BUILDALL, '--base=' + MARLIN_CONFIGS]
    if resume:
        cmd.append('--resume=' + resume)
    if build_opts:
        cmd.extend(build_opts)

    proc = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        cwd=MARLIN_REPO,
    )
    output = proc.stdout + proc.stderr
    # Strip ANSI escape codes so log file is clean
    output_clean = re.sub(r'\x1b\[[0-9;]*m', '', output)
    return proc.returncode, output_clean


def get_failing_config(output):
    """Extract failing config directory from stat file or build output."""
    failing = ''
    if os.path.exists(STAT_FILE):
        try:
            with open(STAT_FILE) as f:
                line = f.read().strip()
            parts = line.split('*')
            if len(parts) >= 2:
                failing = parts[1]
        except (IOError, ValueError):
            pass

    if not failing:
        m = re.search(r'Failed to build\s+(\S+)', output)
        if m:
            failing = m.group(1)

    return failing


def ensure_log_dir():
    """Make sure the .pio directory and log file parent exist."""
    os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)


def log_error(failing, output_tail, extra=""):
    """Append failure info to the shared error log file."""
    ensure_log_dir()
    timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    with open(LOG_FILE, 'a', encoding='utf-8') as f:
        f.write(f"\n{'='*72}\n")
        f.write(f"[{timestamp}] FAILURE: {failing}\n")
        if extra:
            f.write(f"  Note: {extra}\n")
        f.write(f"{'='*72}\n")
        f.write(output_tail)
        f.write("\n")
    print(f"  -> Logged to {LOG_FILE}")


def stash_changes():
    """Stash any uncommitted changes. Returns True if we stashed."""
    result = subprocess.run(
        ['git', 'diff', '--quiet'],
        capture_output=True, cwd=MARLIN_REPO,
    )
    if result.returncode != 0:
        subprocess.run(['git', 'stash'], cwd=MARLIN_REPO, check=True)
        return True
    return False


def restore_stash(stashed):
    """Pop the stash if we created one."""
    if stashed:
        subprocess.run(['git', 'stash', 'pop'], cwd=MARLIN_REPO, check=True)


# ---------------------------------------------------------------------------
# Distributed builds (--hosts)
# ---------------------------------------------------------------------------
OVERLAY_FILES = ['buildroot/bin/build_all_examples', 'buildroot/bin/parallel-build_and_log.py',
                 'buildroot/bin/build_archive']
MARLIN_URL    = 'https://github.com/MarlinFirmware/Marlin.git'
CONFIGS_URL   = 'https://github.com/MarlinFirmware/Configurations.git'
REMOTE_PATH   = 'export PATH="/opt/homebrew/bin:$HOME/.platformio/penv/bin:$HOME/.local/bin:$PATH"'
PROGRESS_RE   = re.compile(r'Getting configuration files from|Logged error|Failed to build|Build succeeded|Iteration|Archiv|Traceback|Error:')


class Host:
    def __init__(self, spec):
        self.spec = spec
        weight = None
        m = re.match(r'^(.*):(\d+)$', spec)
        if m:
            spec, weight = m.group(1), int(m.group(2))
        self.local = spec in ('local', 'localhost')
        self.target = spec
        self.name = 'local' if self.local else spec.split('@')[-1].split('.')[0]
        self.weight = weight
        self.slots = []
        self.rc = None
        self.secs = 0
        self.errors = 0
        self.zips = 0
        self.note = ''

    def argv(self, cmd):
        if self.local:
            return ['bash', '-c', cmd]
        return ['ssh', '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=15',
                '-o', 'ServerAliveInterval=30', self.target,
                'bash -c ' + shlex.quote(cmd)]

    def run(self, cmd, input_file=None, check=True):
        """Run a command here or over SSH. Returns (rc, stdout+stderr)."""
        full = REMOTE_PATH + '; ' + cmd
        for attempt in range(1, 6):
            stdin = open(input_file, 'rb') if input_file else subprocess.DEVNULL
            try:
                proc = subprocess.run(self.argv(full), stdin=stdin, capture_output=True, text=True)
            finally:
                if input_file:
                    stdin.close()
            # ssh itself failed (host asleep or network blip): wait and try again
            if self.local or proc.returncode != 255 or attempt == 5:
                break
            time.sleep(10 * attempt)
        out = (proc.stdout + proc.stderr).strip()
        if check and proc.returncode != 0:
            raise RuntimeError(f"{self.name}: command failed ({proc.returncode}): {cmd[:80]}...\n{out[-600:]}")
        return proc.returncode, out


def keep_awake(h, tries=8):
    """Hold a host awake for the whole run with a long-lived `caffeinate -dis`.

    A sleeping Mac answers ping through its sleep proxy but drops SSH and git
    traffic, so wake it first (retrying) and keep an SSH session open that
    holds the power assertion. Returns the Popen, or None if it never woke."""
    for attempt in range(1, tries + 1):
        p = subprocess.Popen(h.argv('exec caffeinate -dis'), stdin=subprocess.DEVNULL,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(4)
        if p.poll() is None:
            return p
        time.sleep(10 * attempt)
    return None


def git_rev(path, ref='HEAD'):
    r = subprocess.run(['git', 'rev-parse', ref], cwd=path, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"Can't resolve '{ref}' in {path}: {r.stderr.strip()}")
    return r.stdout.strip()


def list_examples(configs):
    """Same list and order as build_all_examples (C-locale sorted unique dirs)."""
    base = os.path.join(os.path.expanduser(configs), 'config', 'examples')
    dirs = set()
    for root, _, files in os.walk(base):
        if 'Configuration.h' in files or 'Configuration_adv.h' in files:
            dirs.add(root)
    return sorted(dirs, key=lambda d: d.encode())


def assign_slots(hosts):
    total = 0
    for h in hosts:
        h.slots = list(range(total, total + h.weight))
        total += h.weight
    return total


def prepare_host(h, a, marlin_sha, configs_sha):
    """Check tools, pin both checkouts, overlay these scripts. Raises on trouble."""
    W = a.work_dir

    # Weight defaults to the core count
    if h.weight is None:
        _, out = h.run('sysctl -n hw.ncpu', check=False)
        h.weight = int(out) if out.isdigit() else 1

    # Preflight: Xcode first-launch / license (a fresh machine fails oddly otherwise)
    rc, out = h.run('xcodebuild -checkFirstLaunchStatus >/dev/null 2>&1; echo $?; command -v git python3 rsync caffeinate >/dev/null || echo MISSINGTOOLS', check=False)
    if 'MISSINGTOOLS' in out:
        raise RuntimeError(f"{h.name}: git, python3, rsync or caffeinate is missing")
    if out.splitlines()[0].strip() != '0':
        raise RuntimeError(f"{h.name}: Xcode needs 'sudo xcodebuild -runFirstLaunch' (and a license check: 'sudo xcodebuild -license accept')")

    # Rosetta: toolchain-atmelavr is x86_64 only, so Apple Silicon hosts need it for AVR examples
    rc, out = h.run('[ "$(uname -m)" != arm64 ] || arch -x86_64 /usr/bin/true >/dev/null 2>&1; echo $?', check=False)
    if out.splitlines()[-1].strip() != '0':
        raise RuntimeError(f"{h.name}: Rosetta is not installed (AVR toolchain fails with 'Bad CPU type'). Run: softwareupdate --install-rosetta --agree-to-license")

    # PlatformIO: install into ~/.platformio/penv if absent
    rc, out = h.run('pio --version 2>/dev/null', check=False)
    if rc != 0:
        spec = f"platformio=={a.pio_version}" if a.pio_version else 'platformio'
        h.run(f'python3 -m venv "$HOME/.platformio/penv" && "$HOME/.platformio/penv/bin/pip" install -q "{spec}"')
        rc, out = h.run('pio --version')
    elif a.pio_version and a.pio_version not in out:
        h.run(f'"$(dirname "$(command -v pio)")/pip" install -q "platformio=={a.pio_version}"')
        rc, out = h.run('pio --version')
    h.note = out.replace('PlatformIO Core, version ', 'pio ')

    # Pinned checkouts
    for sub, url_default, url_over, local_path, sha in (
            ('Marlin', MARLIN_URL, a.fetch_from, MARLIN_REPO, marlin_sha),
            ('Configurations', CONFIGS_URL, a.configs_fetch_from, MARLIN_CONFIGS, configs_sha)):
        url = local_path if h.local else (url_over or url_default)
        d = f'{W}/{sub}'
        unpin = ''
        if sub == 'Marlin':
            unpin = ' '.join(f'git update-index --no-skip-worktree {f} 2>/dev/null;' for f in OVERLAY_FILES)
        h.run(f'mkdir -p "{W}" && ([ -d "{d}/.git" ] || git init -q "{d}") && cd "{d}" && {unpin} '
              f'git reset -q --hard 2>/dev/null; git fetch -q --depth=1 {shlex.quote(url)} {sha} && '
              f'git checkout -q --force --detach FETCH_HEAD && git clean -fdq -e .pio')

    # Overlay this controller's scripts (the pinned SHA may predate --shard)
    for f in OVERLAY_FILES:
        src = os.path.join(MARLIN_REPO, f) if os.path.exists(os.path.join(MARLIN_REPO, f)) else os.path.join(SCRIPT_DIR, os.path.basename(f))
        h.run(f'cat > "{W}/Marlin/{f}" && chmod +x "{W}/Marlin/{f}"', input_file=os.path.join(SCRIPT_DIR, os.path.basename(f)))
    h.run(f'cd "{W}/Marlin" && git update-index --skip-worktree ' + ' '.join(OVERLAY_FILES))


def run_host(h, a, total, marlin_sha, collect, passthrough, lock):
    t0 = time.time()
    W = a.work_dir
    outdir = f'{W}/out-{marlin_sha[:8]}'
    hostdir = os.path.join(collect, h.name)
    os.makedirs(hostdir, exist_ok=True)
    logpath = os.path.join(hostdir, 'run.log')
    shard = ','.join(map(str, h.slots)) + f'/{total}'
    opts = ' '.join(shlex.quote(o) for o in passthrough)
    cmd = (f'cd "{W}/Marlin" && rm -rf "{outdir}" && mkdir -p "{outdir}" && '
           f'export MARLIN_CONFIGS="{W}/Configurations" && '
           f'caffeinate -i python3 buildroot/bin/parallel-build_and_log.py --no-stash '
           f'--shard={shard} --output="{outdir}" {opts}')
    count = 0
    try:
        proc = subprocess.Popen(h.argv(REMOTE_PATH + '; ' + cmd), stdin=subprocess.DEVNULL,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        with open(logpath, 'w') as lf:
            for line in proc.stdout:
                lf.write(line)
                clean = re.sub(r'\x1b\[[0-9;]*m', '', line.rstrip())
                if 'Getting configuration files from' in clean:
                    count += 1
                if PROGRESS_RE.search(clean):
                    with lock:
                        print(f'[{h.name}] {count:4d} {clean[:110]}', flush=True)
        h.rc = proc.wait()

        # Collect archives and the error log
        h.run(f'mkdir -p "{outdir}"')
        os.makedirs(os.path.join(collect, 'archives'), exist_ok=True)
        src = f'{h.target}:{outdir}/' if not h.local else os.path.expandvars(outdir) + '/'
        rs = subprocess.run(['rsync', '-a', src, os.path.join(collect, 'archives') + '/'],
                            capture_output=True, text=True)
        if rs.returncode != 0:
            h.note += f' | rsync failed: {rs.stderr.strip()[:120]}'
        _, errlog = h.run(f'cat "{W}/Marlin/.pio/build_errors.log" 2>/dev/null', check=False)
        h.errors = errlog.count('FAILURE:')
        with open(os.path.join(hostdir, 'build_errors.log'), 'w') as ef:
            ef.write(errlog + '\n')
        _, zc = h.run(f'find "{outdir}" -type f | wc -l', check=False)
        h.zips = int(zc.strip() or 0)
    except Exception as e:  # noqa
        h.rc = h.rc if h.rc else 99
        h.note += f' | {e}'
    h.secs = time.time() - t0


def run_hosts(a, passthrough):
    configs = os.path.expanduser(MARLIN_CONFIGS)
    if not configs or not os.path.isdir(configs):
        sys.exit('Set MARLIN_CONFIGS to a local Configurations checkout.')
    hosts = [Host(x.strip()) for x in a.hosts.split(',') if x.strip()]
    if not hosts:
        sys.exit('--hosts needs at least one host')
    marlin_sha = git_rev(MARLIN_REPO, a.sha or 'HEAD')
    configs_sha = git_rev(configs, a.configs_sha or 'HEAD')
    print(f'Marlin         {marlin_sha[:10]}\nConfigurations {configs_sha[:10]}')
    if a.fetch_from is None and a.configs_fetch_from is None and not all(h.local for h in hosts):
        print('(remote hosts fetch from GitHub: both SHAs must be pushed there)')

    # Resolve weights (core counts) and slots
    for h in hosts:
        if h.weight is None:
            rc, out = h.run('sysctl -n hw.ncpu', check=False)
            h.weight = int(out) if (rc == 0 and out.isdigit()) else 1
    total = assign_slots(hosts)
    examples = list_examples(configs)
    for h in hosts:
        n = sum(1 for i in range(len(examples)) if (i % total) in h.slots)
        print(f'  {h.name:12s} weight {h.weight:3d}  slots {h.slots[0]}..{h.slots[-1]} of {total}  -> {n} of {len(examples)} examples')
    if a.plan:
        return 0

    collect = os.path.expanduser(a.collect or os.path.join(MARLIN_REPO, '.pio', 'hosts-out'))
    os.makedirs(collect, exist_ok=True)
    lock = threading.Lock()

    # Wake every host and hold it awake until the run is over
    awake, live = [], []
    for h in hosts:
        k = keep_awake(h)
        if k is None:
            print(f'[{h.name}] could not be woken or reached over SSH')
            if not a.drop_failed:
                for p in awake:
                    p.terminate()
                return 2
            continue
        awake.append(k)
        live.append(h)
    try:
        return _run_prepared(a, live, total, marlin_sha, configs_sha, collect, passthrough, lock, examples)
    finally:
        for p in awake:
            p.terminate()


def _run_prepared(a, hosts, total, marlin_sha, configs_sha, collect, passthrough, lock, examples):
    # Prepare hosts in parallel; abort before building if any fail
    problems = []
    failed = []
    def prep(h):
        try:
            prepare_host(h, a, marlin_sha, configs_sha)
            with lock:
                print(f'[{h.name}] ready ({h.note})')
        except Exception as e:  # noqa
            with lock:
                problems.append(str(e))
                failed.append(h)
    ts = [threading.Thread(target=prep, args=(h,)) for h in hosts]
    [t.start() for t in ts]
    [t.join() for t in ts]
    if problems:
        print('\nPreparation failed:\n  ' + '\n  '.join(problems))
        if not a.drop_failed or len(failed) == len(hosts):
            return 2
    hosts = [h for h in hosts if h not in failed]
    if sum(h.weight for h in hosts) != total:
        # Some hosts are out: split the examples again over the ones that are ready
        total = assign_slots(hosts)
        print(f'Continuing with {len(hosts)} host(s); new split of {len(examples)} examples:')
        for h in hosts:
            n = sum(1 for i in range(len(examples)) if (i % total) in h.slots)
            print(f'  {h.name:12s} weight {h.weight:3d}  -> {n} examples')

    ts = [threading.Thread(target=run_host, args=(h, a, total, marlin_sha, collect, passthrough, lock)) for h in hosts]
    [t.start() for t in ts]
    [t.join() for t in ts]

    # Merge error logs, tagged by host
    merged = os.path.join(collect, 'build_errors.log')
    with open(merged, 'w') as mf:
        mf.write(f'Distributed build of Marlin {marlin_sha} / Configurations {configs_sha}\n')
        for h in hosts:
            p = os.path.join(collect, h.name, 'build_errors.log')
            if os.path.exists(p):
                mf.write(f'\n##### HOST {h.name} #####\n')
                mf.write(re.sub(r'FAILURE:', f'FAILURE [{h.name}]:', open(p).read()))

    print('\nHost        rc  time     files  failures  notes')
    for h in hosts:
        print(f'{h.name:10s} {h.rc:3d}  {h.secs/60:6.1f}m {h.zips:6d} {h.errors:8d}  {h.note}')
    print(f'\nArchives:   {os.path.join(collect, "archives")}\nError log:  {merged}')
    return 0 if all(h.rc == 0 for h in hosts) else 1


def main():
    parser = argparse.ArgumentParser(
        description='Build all config examples and log failures to a shared log file.'
    )
    parser.add_argument(
        '--loop-limit', type=int, default=0,
        help='Stop after processing N configs (0 = unlimited, default)',
    )
    parser.add_argument(
        '--resume', type=str, default=None,
        help='Start at this config directory name',
    )
    parser.add_argument(
        '--limit', type=int, default=None,
        help='Pass --limit to build_all_examples',
    )
    parser.add_argument(
        '--nofail', action='store_true',
        help='Pass --nofail to build_all_examples (continue on fail)',
    )
    parser.add_argument(
        '--many', action='store_true',
        help='Pass --many to build_all_examples (all envs per config)',
    )
    parser.add_argument(
        '--archive', action='store_true',
        help='Pass -a to build_all_examples',
    )
    parser.add_argument(
        '--tail', type=int, default=TAIL_LINES,
        help=f'Lines of output to log on failure (default: {TAIL_LINES})',
    )
    parser.add_argument('--shard', type=str, default=None,
                        help='Pass --shard=I[,J..]/N to build_all_examples')
    parser.add_argument('--output', type=str, default=None,
                        help='Pass -o to build_all_examples (archive location)')
    parser.add_argument('--no-stash', action='store_true',
                        help='Do not stash local changes (for clean worker checkouts)')
    g = parser.add_argument_group('distributed builds')
    g.add_argument('--hosts', type=str, default=None,
                   help='Comma-separated [user@]host[:weight] list, or "local"')
    g.add_argument('--plan', action='store_true', help='Show the split across hosts and exit')
    g.add_argument('--drop-failed', action='store_true', help='Carry on without hosts that cannot be reached or prepared, and re-split the examples')
    g.add_argument('--sha', type=str, default=None, help='Marlin commit/ref to build (default HEAD)')
    g.add_argument('--configs-sha', type=str, default=None, help='Configurations commit/ref (default HEAD of $MARLIN_CONFIGS)')
    g.add_argument('--work-dir', type=str, default='$HOME/MarlinBuilds-work', help='Work folder on each host')
    g.add_argument('--collect', type=str, default=None, help='Where to gather results (default .pio/hosts-out)')
    g.add_argument('--fetch-from', type=str, default=None, help='Git URL remote hosts fetch Marlin from (default: GitHub; "local" always uses this checkout)')
    g.add_argument('--configs-fetch-from', type=str, default=None, help='Git URL remote hosts fetch Configurations from (default: GitHub)')
    g.add_argument('--pio-version', type=str, default=None, help='Install/require this PlatformIO version on each host')
    args, unknown = parser.parse_known_args()

    if args.hosts:
        passthrough = []
        if args.limit: passthrough.append(f'--limit={args.limit}')
        if args.nofail: passthrough.append('--nofail')
        if args.many: passthrough.append('--many')
        if args.archive: passthrough.append('-a')
        if args.loop_limit: passthrough.append(f'--loop-limit={args.loop_limit}')
        passthrough.extend(unknown)
        sys.exit(run_hosts(args, passthrough))

    # Collect passthrough flags
    build_opts = []
    if args.limit:
        build_opts.append(f'--limit={args.limit}')
    if args.nofail:
        build_opts.append('--nofail')
    if args.many:
        build_opts.append('--many')
    if args.archive:
        build_opts.append('-a')
    if args.shard:
        build_opts.append(f'--shard={args.shard}')
    if args.output:
        build_opts.extend(['-o', os.path.expanduser(args.output)])
    build_opts.extend(unknown)

    # Clear stale stat file
    if os.path.exists(STAT_FILE):
        os.remove(STAT_FILE)

    # Ensure fresh log — back up previous if it exists
    if os.path.exists(LOG_FILE):
        backup = LOG_FILE + '.prev'
        if os.path.exists(backup):
            os.remove(backup)
        os.rename(LOG_FILE, backup)

    ensure_log_dir()
    with open(LOG_FILE, 'w', encoding='utf-8') as f:
        f.write(f"Build error log started at {datetime.datetime.now().isoformat()}\n")

    # Restore stock configs
    subprocess.run(
        [os.path.join(MARLIN_REPO, 'buildroot', 'bin', 'restore_configs')],
        cwd=MARLIN_REPO, check=True, capture_output=True,
    )

    stashed = False if args.no_stash else stash_changes()

    def cleanup(signum=None, frame=None):
        restore_stash(stashed)
        if signum is not None:
            sys.exit(128 + signum)

    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGTERM, cleanup)

    try:
        loop_count = 0
        skip_next = False
        build_args = []
        if args.resume:
            build_args.append(f'--resume={args.resume}')

        for iteration in range(1, 501):
            # Check loop limit
            if args.loop_limit > 0 and loop_count >= args.loop_limit:
                print(f"Loop limit ({args.loop_limit}) reached.")
                sys.exit(0)

            # --- Build ---------------------------------------------------
            build_cmd_args = list(build_args) + build_opts
            # After a logged failure, --skip tells build_all_examples to skip
            # the config recorded in the stat file and continue to the next.
            # Guard: if the stat file has vanished (e.g. external deletion),
            # --skip would cause build_all_examples to exit 0 with "Nothing to
            # skip" — a false success.  Fall back to a fresh run instead.
            if skip_next:
                skip_next = False
                if os.path.exists(STAT_FILE):
                    build_cmd_args = [a for a in build_cmd_args if not a.startswith('--resume=')]
                    build_cmd_args.append('--skip')
                else:
                    print("WARNING: stat file missing before --skip — starting fresh run.")

            print(f"=== Iteration {iteration} {' '.join(build_cmd_args)} ===")

            rc, output = run_build(build_cmd_args)
            print(output)

            # --- Success: all remaining configs built fine ---
            if rc == 0:
                print("Build succeeded for all configs.")
                sys.exit(0)

            loop_count += 1

            # --- Failure: log and skip over the bad config ---
            failing = get_failing_config(output)
            if not failing:
                failing = f"unknown-config-{iteration}"

            output_tail = "\n".join(output.splitlines()[-args.tail:])
            log_error(failing, output_tail)

            # Leave stat file intact so --skip can target this config next call.
            skip_next = True

            print(f"Logged error for {failing}. Moving to next config.")
            print("")

        print("Hit max iterations (500)")
        sys.exit(1)

    finally:
        cleanup()


if __name__ == '__main__':
    main()
