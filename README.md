# Para-Hash

Para-Hash is a new universal hash function focus on AVX and ARMNEON instruction set, using a input block of 256 bits and produce a tag of 256 bits



## Description

A future description of the proyect

## Proyect structure

in the proyect exist two principal folders, one for AVX (X86) and one for ARMNEON (ARM)

src/
 ├── ARM
 └── X86

in each folder exist four principal files

src/
 ├── ARM
    ├──CPB.cpp
    ├──Makefile
    ├──Para-Hash.cpp
    └──Para-Hash.h
 └── X86
    ├──CPB.cpp
    ├──Makefile
    ├──Para-Hash.cpp
    └──Para-Hash.h

The main file of the proyect is CPB.cpp in this file is calculated the Cycles per Byte for the funtions.

## Requeriments

- GCC >= 14
- Make
- Operatin System: Linux / macOS

## Compile 
To compile the proyect it is necesary to enter to the specific folder you going to use, and write the coman make in the console.
After this we going to get a file called test. To execute the code you need to use the comand:

./test file.txt

The program recive a parameter that is the name of the file to get the results.



## Autor

Luis Alejandro Pérez Sarmiento

## Licencia

MIT
