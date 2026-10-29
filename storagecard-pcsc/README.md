# Storage Card Example - PC/SC

This directory contains demonstration code for the `keyple-card-cna-storagecard-cpp-lib` using PC/SC readers. It
shows how to use this Keyple extension library to interact with storage cards (MIFARE Ultralight, MIFARE Classic
1K, ST25/SRT512) through PC/SC contactless readers, combined with standard Calypso card selection.

## Description

This PC/SC-specific demo illustrates the main features of the `keyple-card-cna-storagecard-cpp-lib`:
- Handling of multiple storage card types through PC/SC readers
- Integration with standard Keyple Calypso operations
- Automated card type detection using PC/SC ATR patterns
- Memory operations for various storage cards

## ⚠️ Important: Storage Card Library Requirement

**This demo requires the official `keyple-card-cna-storagecard-cpp-lib` library from Calypso Networks
Association**, fetched automatically by CMake from its private repository. Access to its source is limited to
effective members of the Association.

## Supported Hardware

### Readers
This demo works exclusively with PC/SC contactless readers, such as:
- ASK LoGO
- Any standard PC/SC compliant contactless reader that also supports MIFARE Ultralight and ST25 storage cards

### Supported Cards
The demo supports:
- Storage Cards (via `keyple-card-cna-storagecard-cpp-lib`):
    - MIFARE Ultralight (MFOC, MFOICU1)
    - MIFARE Classic 1K
    - ST Microelectronics ST25 / SRT512
- Standard Calypso cards (via core Keyple)

## Prerequisites

- A C++11-capable compiler (CLang / GCC / MSVC) and CMake 3.5+
- PC/SC middleware installed on your system
- A compatible PC/SC contactless reader
- Access to the official `keyple-card-cna-storagecard-cpp-lib` repository (CNA members only)

## Demo Structure

- `Main_MultiTech_ScheduledSelection_Pcsc.cpp`: configures the PC/SC reader, schedules a multi-technology
  selection scenario (Calypso + the three storage card types), and reports the result through
  `CardReaderObserver`.

## Build and Run

```bash
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain/clang-macos.cmake
cmake --build .
./bin/Main_MultiTech_ScheduledSelection_Pcsc
```

> **Note**: edit `CARD_READER_NAME_REGEX` and `AID` at the top of `Main_MultiTech_ScheduledSelection_Pcsc.cpp` to
> match your actual PC/SC reader name and Calypso application.

## Copyright

Copyright (c) 2026 Calypso Networks Association - [https://calypsonet.org/](https://calypsonet.org/)
