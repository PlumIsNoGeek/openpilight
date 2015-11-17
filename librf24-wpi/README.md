# librf24-wpi

The librf24-wpi is based on https://github.com/bearpawmaxim/librf24-sunxi but is using WiringPi instead of raw GPIO access. This makes it more compatible and not only working on Raspberry Pi but also on Banana Pi boards and all other boards providing a /dev/spi device and wiring pi gpio access.

Wiring Pi for RaspberryPi: http://wiringpi.com/download-and-install/
Wiring Pi for BananaPi: https://github.com/LeMaker/WiringBP/archive/bananapro.zip

1. unzip archive
2. cd into unzipped directory
3. run "./build"
