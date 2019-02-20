# Indoor Localization using CellLocate<span>&#174;

## Abstract

This code is designed to run on the Ublox C027- LISA-U200 board and makes use of the CellLocate feature for indoor position estimation without GPS. When GPS is available, the location is calculated using CellLocate and GPS data in a hybrid mode.

***

## Installation

1) `$sudo apt-get install gdb-multiarch gcc-arm-none-eabi build-essential binutils-arm-none-eabi openocd`

2) `sudo apt-get install python2.7 python-pip`
   
   `pip install pip==8.1.1`

3) `$ sudo pip install mbed-cli`

4) Go to the directory where you want the project to be and import the git repo and:

    `$ mbed import https://github.com/Hameem1/Indoor-Localization-using-CellLocate.git`

5) Plug in the mbed board and then;

    `$ mbed detect`
    
    `$ mbed  toolchain GCC_ARM`
    
    `$ mbed  target UBLOX_C027`
    
6) Change the SIM card configuration in main.cpp

7) To compile the code and deploy to the board;
    
    `$ mbed  compile -f`

    (generates the .bin file in the BUILD folder)
    
    -f causes the .bin file to be uploaded to the board as soon as it’s compiled.

