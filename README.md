# A cross-platform GUI toolkit for DOS based graphical user interfaces

This is an experimental early prototype for a toolkit for (mostly) 16-bit DOS based graphical environments.
The interface is still in fluctuation, and it is probably not going to work well outside a very limited set of examples.
As such **it comes with no warranty**!

Feature set:

* Simple line and text drawing
* Keyboard and mouse input
* Displaying message boxes

Currently supported targets:

* [PC GEM](http://www.deltasoft.com/)
* [DESQview/X](https://www.cs.cmu.edu/afs/cs.cmu.edu/project/phrensy/pub/www/dvx/dvxtechp.htm) (see also [some screenshots](http://toastytech.com/guis/dvx.html))
* 16-bit and 32-bit versions of [Microsoft Windows](https://en.wikipedia.org/wiki/Microsoft_Windows)
* 16-bit and 32-bit versions of [IBM OS/2](https://en.wikipedia.org/wiki/OS%2F2)

All the sources are released under the MIT license, except for the file `external/watcom/cstrtw16.asm` which is based on the Watcom sources and is released under the [Sybase Open Watcom Public License](https://en.wikipedia.org/wiki/Sybase_Open_Watcom_Public_License).
That file is optional for using the toolkit.

## Usage

The project is composed of two parts: the actual libraries and tests to be compiled with it.
To compile the libraries and/or the tests, the [Open Watcom](https://open-watcom.github.io/) cross compiler is required.
The host used for development is a Linux system and requires [GNU Make](https://www.gnu.org/software/make/) (or compatible software) as well.
To compile for PC GEM, the [Watcom port of the OpenGEM SDK](https://github.com/BinaryMelodies/OpenGEM-Open-Watcom) is required.
To compile for DESQview/X, the [Watcom port of the XCB libraries](https://github.com/BinaryMelodies/desqview-xcb) is required.
See below for technical details.

When compiling on Linux (or another UNIX-like system), the `WATCOM` environment variable should be set to the path of the Watcom compiler, all the other variables are set up by the Makefiles.
Issuing `make all` will compile the libraries and the tests.
The libraries and anything required to use the binaries are found under `bin/lib`, while the compiled tests will appear under a system specific directory under `bin`.
Intermediate object files are placed under the directory `obj`.
To compile `external/watcom/cstrtw16.asm`, the files `langenv.inc`, `mdef.inc`, `msgcpryt.inc` and `xinit.inc` from the Open Watcom sources need to be placed under `external`.
These are not included in the repository.

For development, the required header files are found under `src/include`.
A user program only needs to include `api.h`, but the entire path should be included in the include search path.
Furthermore, one of the following directories should also be included in the include search path, depending on the target graphical system:

* For GEM, `src/include/gem` must be included as well
* For DESQview/X, `src/include/dvx` must be included as well
* For Windows, `src/include/win` must be included as well
* For OS/2, `src/include/os2` must be included as well

To make cross platform development easier, several templates exist under `src/include/mfs` that set up the required paths, compilation parameters as well as recommended suffixes.

The sources to the toolkit are found under `src/lib`, and the tests are under `src/tests`.

## Platform specific notes

### Targetting PC GEM

Developed by Digital Research in 1985, it was a single tasking graphical interface for DOS, intended to have a similar look and feel to the Apple Macintosh GUI.
Its features had to be reduced due to a lawsuit faced from Apple, and eventually GEM lost out to Microsoft Windows.
Caldera later bought the assets to GEM and eventually released it under the free and open [GNU GPL 2.0 license](https://en.wikipedia.org/wiki/GNU_General_Public_License).
This has been further developed as FreeGEM and its distribution as OpenGEM, available as a default graphical interface under FreeDOS.

From a technical perspective, GEM evolved from Digital Research's previous GSX graphical libraries.
The API is fairly low level, divided into the low level graphical API provided by VDI, and the windowing API provided by AES.
Other functionality would be provided by the DOS API.
Executable files are in the same format as DOS executables (using the signature `MZ` as the first two bytes) but use the filename extension `.APP` instead of `.EXE`.

The GEM platform is identified as `gem` in most sources.
Targetting it requires the [OpenGEM SDK](https://github.com/BinaryMelodies/OpenGEM-Open-Watcom) to be set up.
It is expected that the GEM include files (including `gembind.h`) are found under `$WATCOM/include/gem`, which must also appear in the include search path.
The GEM SDK libraries `gembnd-s` and `gembnd-l` should appear under `$WATCOM/lib286`.

Two targets are supported: one for small model applications (all pointers are near pointers, with a single separate code and data segment) and one for large model applications (all pointers are far pointers, multiple code and data segments may appear).
The recommended suffix for these applications are `-s` for small model and `-l` for large model.

## Targetting DESQview/X

DESQview has been developed by Quarterdeck Office Systems as a text based windowing multitasking environment for DOS that was compatible with IBM TopView.
Around 1994, DESQview was extended with a graphical interface based on the [X Window System](https://www.x.org/wiki/) that enabled it to run programs that use the X protocol, display UNIX X11 programs running on another computer, and also to translate the graphical output of DOS and Windows programs into X protocol messages and let them display on other X servers.
It eventually lost out to Microsoft Windows.

From a technical perspective, DESQview/X uses the DESQview API, itself an evolution of the predecessor TopView API, with an additional [socket API](https://www.ctyme.com/intr/rb-1726.htm) that lets programs issue socket related calls, required for communicating via the X protocol.
It is agnostic to the executable file format and any DOS program can access these APIs.
A typical executable would use a DOS extender such as [Rational Systems DOS/4GX](https://www.cs.cmu.edu/afs/cs.cmu.edu/project/phrensy/pub/www/dvx/dvxtechp.htm) (itself a modification of the famous DOS/4G), or the early [DJGPP](http://www.delorie.com/djgpp) bindings.
This project uses the more modern [XCB library](https://xcb.freedesktop.org/), compiles to the common DOS .EXE executable format (signature bytes `MZ`) and uses no DOS extender technology.

The DESQview/X platform is identified as `dvx` in most sources.
Targetting it requires the [XCB library](https://github.com/BinaryMelodies/desqview-xcb) to be set up.
It is expected that the DESQview/X include files (including `xcb.h`) are found under `$WATCOM/include/xcb`.
Unlike for other platforms, it is not necessary to have this in the include path, since the header is included as `<xcb/xcb.h>` in the sources.
The XCB libraries `xcblibs` and `xcblibl` should appear under `$WATCOM/lib286`.

Two targets are supported: one for small model applications (all pointers are near pointers, with a single separate code and data segment) and one for large model applications (all pointers are far pointers, multiple code and data segments may appear).
The recommended suffix for these applications are `-s` for small model and `-l` for large model.

## Targetting Windows

Microsoft Windows 1.0 first appeared in 1985 and by all accounts it seems to have been a disappointment.
It originally had tiling windows instead of the more conventional overlapping windows.
Microsoft Windows 2.0 and 2.1 added support for overlapping windows and it came in two editions called Windows/286 (or simply Windows for the 2.0 version) and Windows/386, the latter of which had improved virtualization support for DOS applications.
Microsoft Windows 3.0 and 3.1 reworked a lot of the internals of the system and it is reported to be the first successful version of Windows.
The two editions have been unified, and a "Windows for Workgroups" update was available for 3.10 and 3.11.

At the same time Microsoft started working on a 32-bit version of the system that was intended to be a complete rewrite, more stable and portable to other architectures, called Windows NT.
Eventually 32-bit support was added to Windows 3.1 via the Win32s library.
In 1995, the DOS based product evolved into the 32-bit Windows 95 and definitely displaced all alternatives for PC based graphical user interfaces.

From a technical perspective, Windows introduces a new executable file format (identified by the signature `NE` as the first two bytes) which includes support for multiple segments, resources (including text, graphics and dialog boxes) and dynamically loaded libraries (called DLLs).
32-bit executables have a new file format (identified by the signature `PE`) but similar, expanded features.
All system API is accessed via DLLs, except for the DOS API that is still accessed via interrupts in 16-bit.
Additionally, the 32-bit Windows extender Win386, bundled with Open Watcom, is also supported.
It uses the Phar Lap DOS extender executable format (identified by the signature `MQ`).

The Windows platform is identified as `win` in most sources.
The Windows libraries are included as part of the Open Watcom project.
It is expected that the include search path includes `$WATCOM/include/win` for 16-bit executables (and the Win386 extender) and `$WATCOM/include/nt` for 32-bit executables.

Several targets are supported:

### 16-bit targets

Open Watcom comes with support for the development of Windows 3.1 applications.
The version 2 fork added support for earlier versions of Windows as well.
However, due to the way these systems work, different versions of the startup libraries and/or compiler options are needed to compile for 1.x, 2.x, 3.x.
To overcome this issue, the toolkit is configured in two ways, one that lets applications run on all versions of Windows up to 3.x, and one that lets applications run on Windows 3.x and Windows 95 as well.

Windows 2.x is the first version that checks the required Windows version field in the executable, and if it is larger than 2, it will refuse to run it.
Windows 95 will also check the field and if it is lower than 3, it will refuse to run it, therefore the separate compilations are required.
Windows 3.x also checks the field and if it finds 2, it will issue a warning and execute the program in a backward compatibility environment, unless the memory and font fields of the header file are selected, meaning that the application can use proportionally sized fonts and run in protected mode.

Finally, since Windows 1.x never checks the version field, it will attempt to load any 16-bit executable.
If it tries to include a library function that is not available, it will crash immediately.
To accomodate for this, a modified version of the startup code is provided, `external/watcom/cstrtw16.asm` that tries to dynamically load the library functions if the version of Windows is at least 2.x.
It is recommended that this startup code is used instead of the ones provided by Watcom, however the toolkit will compile and run with the default startup files as well.
The regular Open Watcom startup files can also be used, but they might cause issues with Windows versions they were not compiled for.

### 32-bit targets

There are two ways of compiling 32-bit Windows programs: either compiling as a `PE` executable targetting Windows NT, Windows 95 or the Win32s compatibility layer for Windows 3.1, or compiling as an `MQ` executable that runs via the Win386 Windows extender provided with Open Watcom.

For Win386, the include search path must include `$WATCOM/include/win` as for 16-bit executables, for NT/95/Win32s, it must be replaced with `$WATCOM/include/nt`.

## Targetting OS/2

Microsoft and IBM started collaborating on OS/2 in 1985 after both Microsoft Windows 1.0 and IBM TopView came out to lackluster results.
The initial OS/2 1.0 released in 1987 was text only, but OS/2 1.1 released in 1988 included a graphical API.
The OS/2 1.x versions were all exclusively 16-bit.
Microsoft and IBM split up as Microsoft started working on a portable version of OS/2, eventually transforming into Windows NT, while IBM released the 32-bit OS/2 2.0 in 1992-1993.
This is the only target that does not run on DOS in any of its variants, however due to its history (OS/2 being advertised as a "better DOS than DOS") and support by Open Watcom, it is also included among the compilation targets.

From a technical perspective, OS/2 introduces a new executable file format (identified by the signature `NE` as the first two bytes, originally introduced by Windows), which includes support for multiple segments, resources (including text, graphics and dialog boxes) and dynamically loaded libraries (called DLLs).
32-bit executables have a new file format (identified by the signature `LX`) but similar, expanded features.
All system API is accessed via DLLs.

The OS/2 platform is identified as `os2` in most sources.
The OS/2 libraries are included as part of the Open Watcom project.
It is expected that the include search path includes `$WATCOM/include/os21x` for 16-bit executables and `$WATCOM/include/os2` for 32-bit executables.

## Compiling for multiple targets

To ease compiling the same source code for multiple targets, `include/mfs` contains several Makefile templates that set up environment variables and Make variables.
The recommendation for a project is to copy or link this directory into the main directory of the project, launch the Makefiles from a common Makefile, and create a `Makefile.template` in the main directory that uses the variables.
Useful variables include `$(WCL)` for the name of the compiler, `$(CFLAGS)` and `$(LDFLAGS)` for the compilation and linking options, `$(LNKSFX)` for the result of the linking and `$(EXESFX)` the final executable (typically the two are the same) and `$(VERSION)` for a recommended suffix for the target, in case multiple compilations are done for the specified targets.
Other variables can be checked in the Makefiles, or under `src/tests`.

