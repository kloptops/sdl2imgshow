# SDL2 Image Show

A simple tool for showing an image with some text.

### Usage:

```
Usage: [ -z <config_file>] [ -T <display_template>] [ -F <game_id>] [ -G <option_file.ini>] [ -i <image_file>] [ -a <text_alignment>] [ -f <font_file>] [ -t <text>] [ -c <colour>] [ -P <image_positon>] [ -S <image_stretch>] [ -s <font_size>] [ -p <text_position>] [ -d <shadow_color>] [ -o <shadow_offset>] [ -D] [ -q] [ -k] [ -W] [ -W] [ -O] [ -b <process_name>] [ -x <key=value>]

Command line help:

    -z <config_file>:          config file
    -T <display_template>:     display template for game select mode
    -F <game_id>:              default game selected
    -G <option_file.ini>:      option files file.
    -i <image_file>:           load an image and add it to the stack
    -a <text_alignment>:       set text alignment
    -f <font_file>:            load font.
    -t <text>:                 render text to the display.
    -c <colour>:               set text_color
    -P <image_positon>:        set image_position mode
    -S <image_stretch>:        set image_stretch mode
    -s <font_size>:            set the current font size.
    -p <text_position>:        set text_positon.
    -d <shadow_color>:         sets drop shadow colour, enables drop shadows.
    -o <shadow_offset>:        sets shadow_offset for the drop shadow.
    -D:                        disable drop shadow.
    -q:                        quit immediately mode.
    -k:                        keypress quit mode.
    -W:                        wait quit mode.
    -W:                        quiet mode.
    -O:                        disable font scaling to screen size.
    -b <process_name>:         watch for process_name, quit if it is running.
    -x <key=value>:            set a variable, the value supports variable substitution.
    -X <key=value>:            set a variable, the value doesn't support variable substitution.

```

### INI help:

```ini
image=<image_file>              # Load an image.
image_fallback=<image_file>     # Load an image if the previous image or image_fallback failed to load.
text_position=<position>        # sets the position of images loaded.
image_stretch=<stretch>         # Sets the stretch mode of images loaded.
text_position=<position>        # sets the position of the text rendered to the screen from now on.
screen_margin=<x>,<y>,<w>,<h>   # Sets the margin for anything loaded from now on.
font=<font_file>                # Load a font file.
font_fallback=<font_file>       # Load a font if the previous font or font_fallback failed to load.
font_size=<size>                # Sets the font size, if any fonts are loaded they will be reloaded with this size.
text=<text>                     # Renders text using the current font, font size, font color and shadow settings.
text_color=<r>,<g>,<b>          # Sets the text colour to r,g,b.
shadow_color=<r>,<g>,<b>        # Sets the drop shadow colour to r,g,b. Enables drop shadows.
shadow=<int>,<int>              # Sets the offset of the dropshadow by x/y. Enables drop shadows.
shadow=<bool>                   # Enable/Disable drop shadow for rendered text
quiet=<bool>                    # Enable/Disable wait quiet mode.
quit=<bool>                     # Enable/Disable wait quit mode.
wait_quit=<bool>                # Enable/Disable wait quit mode.
keypress_quit=<bool>            # Enables/Disables keypress_quit mode
set=<key>=<value>               # Sets a variable, allows variable substitution.
set_strict=<key>=<value>        # Sets a variable, does not allow variable substitution.
disable_font_scale=<bool>       # Enables/Disables font scaling to screen height. true = disable
```

### Compile:

```sh
cmake -Bbuild -DCMAKE_BUILD_TYPE="RelDebug"
cmake --build build -j4
```
