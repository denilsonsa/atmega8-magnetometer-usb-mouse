## Introduction ##

This is a USB HID absolute pointing device using an [ATmega8][atmega8] AVR
8-bit microcontroller and a HMC5883L magnetometer. It allows the user to
control the mouse pointer by moving the sensor in the air, pointing it at
the desired position, somewhat similar to the [Wiimote controller][wiimote]
(although using a completely different technology).

[atmega8]: http://www.atmel.com/dyn/products/product_card.asp?part_id=2004
[wiimote]: http://en.wikipedia.org/wiki/Wii_Remote

It was developed by *Denilson Figueiredo de Sá* in the year 2011 as the final
graduating project in order to obtain Bachelor's degree in Computer Science
at [DCC/UFRJ][dcc].

[dcc]: http://www.dcc.ufrj.br/

This project is mirrored at:

* [https://bitbucket.org/denilsonsa/atmega8-magnetometer-usb-mouse][bbmirror]
* [https://github.com/denilsonsa/atmega8-magnetometer-usb-mouse][ghmirror]

[bbmirror]: https://bitbucket.org/denilsonsa/atmega8-magnetometer-usb-mouse
[ghmirror]: https://github.com/denilsonsa/atmega8-magnetometer-usb-mouse

The full text of my thesis (written in Portuguese) is available as PDF in
the download section of [GitHub][ghdownload] and [BitBucket][bbdownload].
The LaTeX source is available in the `monografia/` directory (some minor
tweaking might be needed in order to compile it).

[ghdownload]: https://bitbucket.org/denilsonsa/atmega8-magnetometer-usb-mouse/downloads
[bbdownload]: https://github.com/denilsonsa/atmega8-magnetometer-usb-mouse/downloads

## Photos and videos ##

Photos of the finished project are available at [a Picasa album][picasa].

[picasa]: https://picasaweb.google.com/denilsonsa/Atmega8MagnetometerUsbMouse

You can see this project in action at YouTube:

* [USB Absolute Pointing Device implemented in ATmega8 using
  Magnetometer][ytprototype]
* [Dispositivo apontador com interface USB usando magnetômetro][ytusing]
  (English subtitles available)
* [Moving the mouse pointer using head movements][ythead]
* [Playlist with all videos from this project][ytplaylist] (including
  work-in-progress footage)

[ytprototype]: http://www.youtube.com/watch?v=nZLTwfAJmrE
[ytusing]: http://www.youtube.com/watch?v=lBZV_GAg8yw
[ythead]: http://www.youtube.com/watch?v=1nuw9zsZtk4
[ytplaylist]: http://www.youtube.com/playlist?list=PLA37C87EEDE5EC88C

The schematic diagram of the circuit is available at the `monografia/img/`
subdirectory of this repository.


## How it works ##

The device implements USB HID and should work on any operating system (has
been successfully tested on Linux, Mac OS X and Windows). It identifies
itself as a keyboard and a mouse (actually, an "absolute pointing device").

It has a physical switch that selects between two modes of operation
(*configuration mode* and *mouse mode*) and three push-buttons.

Upon plugging the device to the computer, the user should set the switch to
*configuration mode* and open any simple text editor. In this mode, the
device prints the configuration menu by sending (virtual) keyboard events
to the computer (maybe it would be more accurate to say that it "types" the
menu items, instead of printing). Two of the device buttons are used to
navigate the menu items (selecting the next or the previous item), and the
third button confirms the current selection.

Once in the *configuration mode*, the user should calibrate the "zero" from
the sensor, as well as the screen corners. The calibration data is stored
in the EEPROM memory of the microcontroller, and thus it will be remembered
even after unplugging the device.

Upon starting the "zero" calibration, the device will start printing values
from the sensor, and the user should move the sensor in all possible
directions, trying to obtain the maximum and minimum values for each of the
three axes (X, Y, Z). The *confirm* button should be pressed to finish the
calibration. This calibration is required because the sensor might have a
bias and thus return values that are not centered on number zero (see
images `zerocal_off` and `zerocal_on` from `monografia/img/` subdirectory).

After the "zero" is correctly calibrated, the user should calibrate each
screen corner. The user should navigate the menu items up to `Set topleft`,
point the sensor at top-left corner of the screen and then press the
*confirm* button. This should be repeated for all other corners. For best
results, the user should be directly in front of the screen center, and the
screen should be facing either to the North or to the South direction.

The "zero" calibration should be needed only once, right after building the
project. The corner calibration, on the other hand, is required anytime the
user faces a different screen orientation.

After completing these two calibrations, the device is ready, and the user
may switch to *mouse mode*. In this mode, the mouse pointer will be moved
according to the movements of the sensor, and the three buttons work as
mouse buttons (left, right and middle button).

The device reads the magnetic field measurements from the sensor as a
3-axis vector and applies an algorithm to convert that 3D vector into 2D
screen coordinates. For details about the algorithm, read the `mouseemu.c`
source code.

Due to the limited sensor precision and the amount of captured noise, the
device applies a smoothing filter to the pointer position. This increases
the perceived precision, but also introduces a slight delay in the
movements.

The mode switch can also be used to pause the mouse position, as the
pointer is not moved while in the *configuration mode*.

All the steps mentioned here can be seen in [this video][ytusing].


## Possible improvements ##

* Use a microcontroller with more memory. This is needed before
  implementing any further improvements.

* Use the `DRDY` interrupt signal from the sensor in order to achieve up to
  160Hz. The currently implemented method uses a 75Hz continuous
  measurement mode together with polling. It was implemented this way
  because [the sensor PCB I bought from eBay][fromebay] did not have the
  `DRDY` line available. [The PCB being sold at Love Electronics][fromle]
  has that line.

* For best results, the user must be facing to the North or to the South
  direction. If, instead, the user is facing to the West or to the East,
  the vertical movement of the pointer is severely degraded. This happens
  because, in this case, the sensor rotates around the same axis as the
  magnetic field, and thus gives little to no change in the measurements.
  A solution for this problem is to attach an accelerometer as a second
  sensor to this device.
  
    * With these two sensors, the magnetometer can be used for horizontal
      pointer movement and the accelerometer for the vertical pointer
      movement.

    * These two sensors can be used together to implement a
      tilt-compensation (similar to [this tutorial from Love
      Electronics][letilt]).

    * A third sensor, gyroscope, can be added in order to improve precision
      and reduce the pointer shaking, increasing the responsiveness of the
      device.

* Try another magnetometer with better precision (if there is such thing).

* Try other algorithms for converting the coordinates.

* Implement wireless communication between the device and the computer.

    * It can be done by using a pair of microcontrollers: one next to the
      computer, talking to the USB port; and another next to the sensor.
      The communication between these two microcontrollers can be wireless.
      This solution has been done before in [two][wishabi] [other][usbwtm]
      projects.

    * Or it can be done by implementing a Bluetooth HID device.

[fromebay]: http://cgi.ebay.com/HMC5883L-HMC5883-Triple-Axis-Magnetometer-Sensor-board-/260770948278
[fromle]: https://www.loveelectronics.co.uk/products/140/3-axis-magnetometer---hmc5883-breakout-board
[letilt]: https://www.loveelectronics.co.uk/Tutorials/13/tilt-compensated-compass-arduino-tutorial
[wishabi]: http://vusb.wikidot.com/project:wishabi
[usbwtm]: http://instruct1.cit.cornell.edu/courses/ee476/FinalProjects/s2010/ss868_jfe5/ss868_jfe5/


## Requirements ##

In order to build this project, you need:

* **ATmega8** or any other similar AVR microcontroller. If using a
  different model, some minor fine-tuning of the firmware might be neded.
  By the way, if you are going to buy a microcontroller, I highly recommend
  choosing one with more memory. Although 8KiB was enough, some parts of
  the firmware had to be disabled in order to fit. If you can, get a device
  with at least 16KiB of flash memory.
* **HMC5883L** 3-axis magnetometer. If you use a different sensor, be
  prepared to rewrite the sensor handling code.
* Other electronic components. See the circuit schematic at
  `monografia/img/AVR-magnetometer-usb-mouse`, available in SVG, PNG and
  PDF formats.

The required software environment:

* [AVR-GCC][avrgcc] - Developed with version 4.5.3. Different versions
  require updating a few compiler flags at the `Makefile`, as the available
  flags change between each major version.
* [AVR-Libc][avrlibc] - Developed with version 1.7.0.
* [AVRDUDE][avrdude], or any other tool to write the firmware onto the
  device.
* Unix-like system - Developed on Gentoo Linux amd64, should work anywhere
  with the standard Unix tools.

[avrgcc]: http://gcc.gnu.org/install/specific.html#avr
[avrlibc]: http://www.nongnu.org/avr-libc/
[avrdude]: http://www.nongnu.org/avrdude/


## Directories in this repository ##

The main contents of this project are in these three directories:

* `firmware/` - Contains the source-code of the firmware.
* `projection/` - Python code for studying different algorithms for
  converting the 3D vectors to 2D screen coordinates.
* `monografia/` - LaTeX source of the thesis (written in Portuguese).
* `apresentacao/` - LaTeX source of the presentation (written in Portuguese).

There are also some extra directories:

* `html_javascript/` - Fancy way to show the pointer position using HTML5
  and WebGL.
* `linux_usbhid_bug/` - Information about a minor bug in Linux USB HID
  handling.
* `other_scripts/` - Some scripts to generate a graph of the firmware size
  over time.


## How to build this project ##

All commands listed here assume you are inside the `firmware` directory (the
one with `Makefile` and `checksize`).

Want a quick list of available *make targets*? Run `make help`.


### Preparation ###

These steps only need to be done once. They are the initial setup of the
project.

1. Mount the hardware on your breadboard.
   You can find a short description at the *Hardware description* comment in
   `main.c` file and a complete circuit schematic at
   `monografia/img/AVR-magnetometer-usb-mouse`, available in SVG, PNG and
   PDF formats.

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
    6. Set `ENABLE_KEYBOARD`, `ENABLE_MOUSE` and `ENABLE_FULL_MENU` to `1` or
       `0`, according to what you want in the final firmware. Look at the
       comments in that file for detailed information.

5. Run `make writefuse` to write the fuse bits.

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
microcontroller (see the section about the bootloader).

1. Run either `make all` or `make combine`.
    * `make` is a shortcut for `make all`.

    * `make combine` uses some special compiler flags in order to compile
      all files at the same time, leading to extra optimizations not
      possible when compiling separately. This command will not work on
      GCC 4.6 or newer, because the flags have changed (and, thus, they
      need to be updated). Read `Makefile` to learn more.

    * If it fails, try running `make clean`. The `Makefile` from this
      project is not perfect and does not list all file dependencies. It's
      always a good idea to run `make clean` whenever something fails.

2. Run `make writeflash`.

After you edit the firmware, you only need to redo these two steps.


## Acknowledgements ##

* *Prof. Nelson Quilula Vasconcelos*, advisor for this project.
* *Bruno Bottino Ferreira* for the help and patience during this project.
* *Marcelo Salhab Brogliato* for suggesting the coordinate conversion using
  linear equations.
* *OBJECTIVE DEVELOPMENT Software GmbH* for the awesome [V-USB][vusb]
  firmware-only implementation of USB for AVR devices.
* *Atmel Corporation* for the AVR microcontrollers and the [AVR315: TWI
  Master Implementation][avr315].
* Authors and contributors of all open-source and free software used during
  this project.

[vusb]: http://www.obdev.at/products/vusb/index.html
[avr315]: http://www.atmel.com/dyn/products/documents.asp?category_id=163&family_id=607&subfamily_id=760


 vi:expandtab:filetype=markdown
