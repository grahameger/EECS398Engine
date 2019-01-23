# EECS398Engine
Search Engine Project for EECS 398 W19 at the University of Michigan

## Folder Structure

------

`bin` All output executables, both for the app and for any tests and spikes.

`build`All object files. Removed on `make clean`

`doc` Any notes, configurations files, etc.

`include` All project header files. (`*.h, *.hpp`)

`lib` Any libraries that get compiled by the project or needed in development (don't think we'll have much use for this, but OpenSSL might have it.)

`spike` Small classes, test files, things currently in the early stages of development but you still want to have them committed to the repo on `master` branch.

`src` The program source files. No libraries `*.cpp` only.

`test` All test code files.



## Makefile

Will start by just using a standard Makefile that works on Linux and Windows. If we need to do any real dirty platform specific stuff then we can use `autotools` but I don't see the need quite yet.

## Style

The .astylerc file from Piazza is in the folder root. It should be run from the terminal before pushing to master.

TODO: automate style checking on git push.

## Design Patterns

Should be an evolving thing but to start will be designing everything as self contained libraries. Header files in the `include` directory, Source in the `src` directory. Everything will compile easier with a Makefile.

**TODO** decide whether we want multiple executables (crawler, index, server, etc) or one executable that can do everything and is configurable from the command line.

