TODO: improve this README file...

## How to build this project ##

So you want to build this project? Let's build it together, step-by-step!

Want a quick list of available *make targets*? Run `make help`.

All commands listed here assume you are inside the `firmware` directory (the
one with `Makefile` and `checksize`).

### Preparation ###

These steps only need to be done once. They are the initial setup of the
project.

1. Mount the hardware on your breadboard.
   You can find a short description at the *Hardware description* comment in
   `main.c` file. Hopefully, in future, there will be a nice SVG and PNG
   circuit schematic.

2. Open `hardwareconfig.h` and check if those definitions are consistent with
   the hardware. Basically, just check if the USB D- and USB D+ are connected
   to the correct pins.

3. Open `TWI_Master.h` and check if `TWI_TWBR` value is correct. It should be
   updated if you use a different clock rate.

4. Open `Makefile`.
    1. Set `AVRDUDE_PARAMS` according to your AVR programmer, if you use
       something other than USBasp.
    2. If you use a clock other than 12MHz, update `F_CPU` setting.
    3. If you use a microcontroller other than ATmega8, update `GCC_MCU`,
       `AVRDUDE_MCU`, `BOOTLOADER_ADDRESS` and `CHECKSIZE_CODELIMIT`.
    4. Also check if the fuse bits from `AVRDUDE_PARAMS_FUSE` are correct.
    5. If you want to use a bootloader, set `BOOTLOADER_ENABLED` to `1`. Make
       sure your device has enough space to hold the main firmware together
       with the bootloader.
    6. Set `ENABLE_KEYBOARD` and `ENABLE_MOUSE` to `1` or `0`, according to
       what you want in the final firmware. Remember that 8KiB of ROM is not
       big enough to hold both features together.

5. Run `make writefuse` to write the fuse bits.

### More information about `ENABLE_KEYBOARD` and `ENABLE_MOUSE` ###

If your hardware has enough ROM (Flash) space to hold the firmware with those
two options enabled together, just enable them both and have fun!

However, if your hardware has only 8KiB of ROM (e.g. ATmega8), then you can't
enable both. So, you should first compile a firmware with `ENABLE_KEYBOARD=1`
and `ENABLE_MOUSE=0`, write it to the microcontroller and use the built-in
menus to setup the calibration values. After that is done, rebuild the
firmware with `ENABLE_KEYBOARD=0` and `ENABLE_MOUSE=1`. This second firmware
contains the useful code that emulates a mouse, while the former one had only
the configuration/debugging/calibration code.

### Writing the bootloader (optional) ###

This section is completely optional. You don't need a bootloader. It's just
cool and handy, but you don't need it. Feel free to skip these steps.

This project comes with USBaspLoader. After it is written to the
microcontroller, any later firmware update can be done without the need of a
dedicated AVR programmer.

After the bootloader is written, if a certain condition is true (a specific
button is held down) during the device boot, then the bootloader will take
control and the device will identify itself as USBasp. Writing to this
"virtual" USBasp will actually update the firmware, without the need of any
extra hardware.

1. Did you update the `Makefile` as described above? Did you run `make
   writefuse`?

2. Run `make clean`.

3. Run `make boot`. This will compile the bootloader.

4. Run `make writeboot`. This will write the bootloader to the microcontroller.
   You need an AVR programmer for this step.

5. Run `make clean` to clean up compiled files. This is required because the
   compiled files from the bootloader are incompatible with the main project
   (and vice-versa).

All done! You don't need an AVR programmer anymore!

### Writing the EEPROM (optional) ###

You don't need to write the EEPROM now. You can just use the firmware's
built-in menus (enabled with `ENABLE_KEYBOARD`) to interactively update the
settings stored in the EEPROM.

The EEPROM values defined in `sensor.c` are appropriate for my sensor.
Probably your sensor will have different calibration numbers, and thus it is
highly recommended to use the firmware's menus to calibrate it (at least
once).

Anyway, to write the EEPROM values, just run `make`, followed by `make
writeeeprom`.

### Writing the main firmware ###

You either need an AVR programmer, or you need to start the bootloader on the
microcontroller (see the previous section).

1. Run `make` (or `make all`, which is exactly the same thing).
    * If it fails, try running `make clean`. This `Makefile` is not perfect
      and does not list all file dependencies. It's always a good idea to run
      `make clean` whenever something fails.

    * Note: you may prefer to run `make combine` instead of `make all`. It
      uses some extra compiler flags to compile all source files at the same
      time, which usually results in a smaller firmware. Should work in GCC
      4.5, but won't work on GCC 4.6 (a different set of flags is needed, read
      the `Makefile` for more details).

2. Run `make writeflash`.

After you edit the firmware, you only need to redo these two steps.


That's all, folks!

And have fun!



 vi:expandtab:filetype=markdown
