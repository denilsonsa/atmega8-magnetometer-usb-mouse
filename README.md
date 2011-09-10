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

3. Open `TWI_Master.h`.
    1. Check if `TWI_TWBR` value is correct. It should be updated if you use a
       different clock rate.
    2. Check if `TWI_BUFFER_SIZE` is big enough. Unless you modify the
       firmware, the value here don't need to be changed.

4. Open `Makefile`.
    1. Set `AVRDUDE_PARAMS` according to your AVR programmer, if you use
       something other than USBasp.
    2. If you use a clock other than 12MHz, update `F_CPU` setting.
    3. If you use a microcontroller other than ATmega8, update `MCU`,
       `PROGRAMMER_MCU`, `BOOTLOADER_ADDRESS` and `CHECKSIZE_CODELIMIT`.
    4. Also look at the `writefuse` target. Check if the value of those fuse
       bits make sense for you (and for your hardware).

5. Run `make writefuse` to write the fuse bits.

### Writing the bootloader (optional) ###

This section is completely optional. You don't need a bootloader. It's just
cool and handy, but you don't need it. Feel free to skip these steps.

This project comes with USBaspLoader. After it is written to the
microcontroller, and any later firmware update can be done without the need of
a dedicated AVR programmer.

After the bootloader is written, if a certain condition is true (a specific
button is held down) during the device boot, then the bootloader will take
control and the device will identify itself as USBasp. Writing to this
"virtual" USBasp will actually update the firmware, without the need of any
extra hardware.

1. Run `make clean`.

2. Run `make boot`. This will compile the bootloader.

3. Run `make writeboot`. This will write the bootloader to the microcontroller.
   You need an AVR programmer for this step.

4. Run `make clean` to clean up compiled files. This is required because the
   compiled files from the bootloader are incompatible with the main project
   (and vice-versa).

All done!

### Writing the main firmware ###

You either need an AVR programmer, or you need to start the bootloader on the
microcontroller (see the previous section).

1. Run `make`. If it fails, try running `make clean`.

2. Run `make writeflash`.

After you edit the firmware, you only need to redo these two steps.


That's all, folks!

And have fun!



 vi:expandtab:filetype=markdown
