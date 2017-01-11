# Psiora Emulator Installation

Psiora is an emulator of the Psion Organiser II.
Many models, datapaks and rampaks are supported.

Psiora runs on Linux, Windows.
It should in theory work on OSX, but has not been tested.
Please let me know if it works for you.

## Requirements

Psiora requires SDL 2.0.5 and QT5

## Compiling (Windows and Linux)

If you are using windows you may find it easier to install QtCreator to compile the "psiora.pro" file.
The code has been created to work with either the MSVC or GCC version of the compiler and libraries.
You may need to change the paths in "psiora.pro" to your library paths on windows.

Alternatively you can build the program with qmake.

```bash
git clone https://github.com/scottrichardscg/psiora.git
qmake psiora.pro
make all
```

## Installation on Linux

Copy the "psiora" executable to a location of your choice.
Then run "psiora" to start the emulator.

## Installation on Windows

Copy the "psiora.exe" to a location of your choice. You will also require the QT DLLs
to be included in the same location or in your PATH.
Additionally SDL2.DLL will need to be in the psiora.exe. (Available from libsdl.org)

## Feedback and Bug Reports

Please contact me with any feedback or bugs. Alternatively you can use the ticket system to submit problems.
