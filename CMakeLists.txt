# *****************************************************************************
# Copyright (c) 2026 Calypso Networks Association https://calypsonet.org/     *
#                                                                             *
# See the NOTICE file(s) distributed with this work for additional            *
# information regarding copyright ownership.                                  *
#                                                                             *
# This program and the accompanying materials are made available under the    *
# terms of the Eclipse Distribution License 1.0 which is available at         *
# https://www.eclipse.org/org/documents/edl-v10.php                           *
#                                                                             *
# SPDX-License-Identifier: BSD-3-Clause                                       *
# *****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(KeypleCnaCppExample C CXX)

SET(CMAKE_MACOSX_RPATH 1)
SET(CMAKE_CXX_STANDARD 11)

# Compilers
SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)

IF(NOT TOOLCHAIN_FILE)
    MESSAGE(FATAL_ERROR "Toolchain file not specified")
ENDIF()

# Set common output directory
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add projects
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/storagecard-pcsc)
