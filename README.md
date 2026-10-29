# keyple-cna-cpp-example

This repository contains demonstration code for Calypso Networks Association (CNA) Keyple C++ libraries.

## Examples

- [`storagecard-pcsc`](./storagecard-pcsc): storage card detection (MIFARE Ultralight, MIFARE Classic 1K,
  ST25/SRT512) combined with Calypso card selection, through PC/SC contactless readers.

## Build the Code

Each example is built with CMake; dependencies are resolved via CMake's `FetchContent`, downloaded at configure
time. A toolchain file is required for every build; see `toolchain/` for the profiles used in CI (`gcc-linux`,
`clang-macos`, `clang-windows`, etc.), for example:

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/clang-macos.cmake
cmake --build .
```
