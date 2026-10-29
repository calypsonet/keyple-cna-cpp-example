# *****************************************************************************
# Copyright (c) 2026 Calypso Networks Association https://calypsonet.org/     *
#                                                                             *
# Access, distribution and usage restricted to effective members of the      *
# Calypso Networks Association.                                              *
#                                                                             *
# This program and the accompanying materials are made available under the   *
# terms of the CNAML - (Calypso Networks Association Member License).        *
#                                                                             *
# SPDX-License-Identifier: LicenseRef-CNAML                                   *
# *****************************************************************************/

SET(EXECUTABLE_NAME Main_MultiTech_ScheduledSelection_Pcsc)

INCLUDE_DIRECTORIES(

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main
)

# Add necessary deps
INCLUDE("CMakeLists.txt.keyple-util")
INCLUDE("CMakeLists.txt.keypop-reader")
INCLUDE("CMakeLists.txt.keypop-calypso-card")
INCLUDE("CMakeLists.txt.keypop-storagecard")
INCLUDE("CMakeLists.txt.keyple-service")
INCLUDE("CMakeLists.txt.keyple-card-calypso")
INCLUDE("CMakeLists.txt.keyple-plugin-pcsc")
INCLUDE("CMakeLists.txt.keyple-card-cna-storagecard")

ADD_EXECUTABLE(

    ${EXECUTABLE_NAME}

    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/CardReaderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/Main_MultiTech_ScheduledSelection_Pcsc.cpp
)

TARGET_LINK_LIBRARIES(

    ${EXECUTABLE_NAME}

    Keyple::Util
    Keypop::Reader
    Keypop::Calypso::Card
    Keypop::StorageCard
    Keyple::Service
    Keyple::Card::Calypso
    Keyple::Plugin::Pcsc
    Keyple::Card::Cna::StorageCard
)
