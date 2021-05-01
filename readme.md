# libwotpp
Interface with [wot++](https://github.com/Jackojc/wotpp) in C++!
## Build
This project comes with a handy set of scripts that automatically
clone the official wot++ repository, and apply some modifications to
it such that it becomes suitable to be built as a library.
### Generating the source
You will need to install the `parse` python library.
```sh
pip install parse
```
The following set of scripts should work automatically. In case
they don't, that entitles you to the right to open an issue in this
repo.
```sh
./recreate.sh
./fix_includes.sh
```
### Compiling
Assuming all the sources have been generated correctly you can now
build this project normally as a meson/ninja project.
```sh
meson build --buildtype=release
```
**NOTE**: If you want to set a custom compiler, remember to set
both the CXX and CC variables.
```sh
CXX=clang++ CXX_LD=lld CC=clang CC_LD=lld meson build
--buildtype=release
```
Build and install:```sh
ninja -C build
```
```sh
ninja -C build install
```
And you should be good to go.
