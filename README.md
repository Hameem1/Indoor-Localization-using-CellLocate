# Indoor Localization using CellLocate<span>&#174;

## Abstract

This code is designed to run on the Ublox C027- LISA-U200 board and makes use of the CellLocate feature for indoor position estimation without GPS. When GPS is available, the location is calculated using CellLocate and GPS data in a hybrid mode.

***

## Installation

1) `$ sudo apt-get install gdb-arm-none-eabi gcc-arm-none-eabi build-essential`

2) `$ sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi`

3) `$ sudo pip install mbed-cli`

4) Go to the directory where you want the project to be and import the git repo and:

    `$ insert command`

5) Plug in the mbed board and then;

    `$ mbed detect`
    
    `$ mbed  toolchain GCC_ARM`
    
    `$ mbed  target UBLOX_C027`

6) To compile the code and deploy to the board;
    
    `$ mbed  compile -f`

    (generates the .bin file in the BUILD folder)
    
    -f causes the .bin file to be uploaded to the board as soon as it’s compiled.



