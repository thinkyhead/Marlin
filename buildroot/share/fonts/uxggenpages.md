
Language engine and tools
=========================

Supported hardware
------------------

Marlin supports HD44780 character LCD and 128x64 graph LCD via U8GLIB.
Because of the limitation of the HD44780 hardwares, Marlin can only supports three charsets in those hardwares:
Japaness(kana_utf8), Russian/Cyrillic(ru), or Western(English or western languages)

For the graphic LCD modules, there's no limitation as before. User or language maintainer can easily
change the language translation strings if they save it as UTF-8 encoded files. The only extra step is
to regenerate the font data from a existing BDF font file.


How to support a new language?
------------------------------
1. prepare the translation source file

You need to add a language header file language_xx.h,
where xx is the language/country code, for example en is for English.
You may start with a existing language file by copying it to a new location:

```bash
cp language_zh_CN.h language_zh_TW.h
```

Then you can replace the strings in the macro _UxGT().

Make sure the file is saved as UTF-8 encoded.


2. generates the font data file

This step is to gather the glyph appears in the language file from a 9pt bitmap font,
and save it in a language_data_xx.h file. So the font for converting should contains all
of the characters in your language file.

The default bitmap font is WQY's 9pt bitmap font. You may download from

http://wenq.org/daily/wqy-bitmapfont-bdf-gb18030-nightly_build.tar.gz

or install in Debian/Ubuntu system:

```
sudo apt-get install xfonts-wqy
```


There's a script file in the source code folder `Marlin', which will generate the font data for
all of the language translation files. You may edit the script to change the variable
LANGS to the list of languages you want to process. For example

```bash
LANGS="zh_TW"
```

and then you run the script to get the font data (language_data_xx.h):

```bash
cd Marlin/
../buildroot/share/fonts/genallfont.sh
```

3. change the language settings

Now, to compile the firmware for your language, you need change the
language setting in file Configuration.h.

```cpp
#define LCD_LANGUAGE zh_TW
```

4. compile and update the firmware

Open the Marlin.ino in the IDE and compile the firmware.


Update the language translation
-------------------------------
If you change the language files, you need to run the script genallfont.sh
again to update the font data file.


Use a cool font
---------------
You may need to use a different font to support your own language,
because the default 9pt font is not complete.
(You may support them by adding missing glyph to the font.)

After you prepared your font, you need to specify the font file path as an argument for
script buildroot/share/fonts/genallfont.sh, so that the font it used for your language
is your new font. For example, your new font file name is newfont.bdf, then run the
following command.

```bash
cd Marlin/
../buildroot/share/fonts/genallfont.sh ./newfont.bdf
```


Suggestions for Maintainer
--------------------------
Even the tool and the language engine can be easily updated,
since it use the common bitmap font files and UTF-8 text,
the maintainer need to confirm that the fonts contains the glyph
in the language files.

The font file currently used to generate the font data is the file ISO10646-0-3.bdf,
which is combined with the Marlin's ISO10646-1 fonts and WQY 9pt bitmap font.

Related resources
-----------------

Fonts
WQY: http://wenq.org/

WQY 9pt bitmap font: http://wenq.org/daily/wqy-bitmapfont-bdf-gb18030-nightly_build.tar.gz

unifont (16x8 or 16x16): http://unifoundry.com/unifont.html


The following are some documents related to the old version of language engine:

http://www.marlinfw.org/docs/development/fonts.html

https://github.com/MarlinFirmware/Marlin/wiki/LCD-Language

U8GLIB: https://github.com/olikraus/u8glib.git

UTF-8 for U8GLIB: https://github.com/yhfudev/u8glib-fontutf8.git

Standalone test project for the UTF-8 language engine of Marlin: https://github.com/yhfudev/marlin-fontutf8.git

