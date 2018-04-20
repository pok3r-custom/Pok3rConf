# Pok3rConf

**NOTE: This tool is currently a work in progress, and may not do anything! See [pok3rtool](https://github.com/pok3r-custom/pok3rtool) for flashing firmware from the command line.**

Pok3rConf is a GUI tool for Vortexgear and similar keyboards, including the POK3R, POK3R RGB, Vortex Core, and others. It is intended as a utility for replacing the firmware on supported keyboards with the [qmk_pok3r](https://github.com/pok3r-custom/qmk_firmware) firmware, and configuring the firmware on-the-fly.

Planned features include GUI-configurable layers, keymaps and backlighting.

## Installation
When a release is made, an installer can be found on the [releases](https://github.com/pok3r-custom/Pok3rConf/releases) page.

## Building
Required: `qt5 cmake`
```
git clone --recurse-submodules https://github.com/pok3r-custom/Pok3rConf.git pok3rconf
mkdir pok3rconf-build
cd pok3conf-build
cmake ../pok3rconf
make
```
To edit with Qt Creator, open from the `CMakeLists.txt`.
