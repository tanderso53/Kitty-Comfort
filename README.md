# Kitty Comfort #

## Introduction ##

Kitty Comfort is a program using Arduino compatible microcontrollers
to measure ammonia and environmental systems around a cats litter box,
then triggering an air purification system when the air ammonia levels
are too high. In this way, optimal house cat comfort can be acheived.

Kitty Comfort includes the Kitty Comfort microcontroller program, the
Kitty Comfort database and dashboard solution, and the kittyfiler data
filing program.

## License ##

Kitty Comfort is copyright Tyler J. Anderson, 2021, but is Free and
Open Source Software under the BSD 2-clause license. Source code from
other Free and Open Source project was also used in Kitty Comfort and
Kitty Filer under the terms of the licenses of the respective
subcomponents. See LICENSE for details.

## Dependencies ##

Kitty Comfort requires the following:

- An arduino-compatible microcontroller board with appropriate sensors
- The arduino IDE and libraries

Kittyfiler requires the following additional software:

- jsoncpp
- libpq
- pqxx
- GNU make
- clang

GCC can be used instead of clang by passing GCC as the value of the CXX
variable in the kitty filer makefile.

## Building and Installing ##

For Kitty Comfort:

Use the arduino IDE or arduino-ce to compile and upload the Arduino sketch
to the microcontroller.

For Kittyfiler:

Note that on some systems (BSDs) GNU make is not the default and the command
`gmake` will be required instead of `make` used below.

```sh
cd kittyfiler # Enter the subproject's directory
make # Compile the source
sudo make install # Install to the default system path
```

## Kittyfiler ##

To use kittyfiler use the following command, replaceing `yourserialhere0`
with the serial device connected to the arduino.

```sh
kittyfiler -f outfile.csv /dev/yourserialhere0
```

This will read data from the arduino and dump it in the given CSV file.
It can also dump to a Postgresql database that has been set up if the
command is given as follows.

```sh
kittyfiler -b -d yourdatabase -u youruser -P yourpassword /dev/yourserialhere0
```

Note the connection string elements need to be with the ones for your database
instance.

The program will continue looping until it receives the interupt signal, `^c`,
then it will exit to the command prompt.
