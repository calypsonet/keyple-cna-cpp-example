/* ****************************************************************************
 * Copyright (c) 2026 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * Access, distribution and usage restricted to effective members of the      *
 * Calypso Networks Association.                                              *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the CNAML - (Calypso Networks Association Member License).        *
 *                                                                            *
 * SPDX-License-Identifier: LicenseRef-CNAML                                  *
 **************************************************************************** */

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "calypsonet/keyple/card/storagecard/StorageCardExtensionService.hpp"
#include "keyple/card/calypso/CalypsoExtensionService.hpp"
#include "keyple/core/service/Plugin.hpp"
#include "keyple/core/service/SmartCardService.hpp"
#include "keyple/core/service/SmartCardServiceProvider.hpp"
#include "keyple/core/util/cpp/Logger.hpp"
#include "keyple/core/util/cpp/LoggerFactory.hpp"
#include "keyple/plugin/pcsc/PcscCardCommunicationProtocol.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactoryBuilder.hpp"
#include "keyple/plugin/pcsc/PcscReader.hpp"
#include "keypop/calypso/card/CalypsoCardApiFactory.hpp"
#include "keypop/calypso/card/card/CalypsoCardSelectionExtension.hpp"
#include "keypop/reader/CardReader.hpp"
#include "keypop/reader/ConfigurableCardReader.hpp"
#include "keypop/reader/ObservableCardReader.hpp"
#include "keypop/reader/ReaderApiFactory.hpp"
#include "keypop/reader/selection/BasicCardSelector.hpp"
#include "keypop/reader/selection/CardSelectionManager.hpp"
#include "keypop/reader/selection/IsoCardSelector.hpp"
#include "keypop/storagecard/MifareClassicKeyType.hpp"
#include "keypop/storagecard/StorageCardApiFactory.hpp"
#include "keypop/storagecard/card/ProductType.hpp"
#include "keypop/storagecard/card/StorageCardSelectionExtension.hpp"

#include "CardReaderObserver.hpp"

using calypsonet::keyple::card::storagecard::StorageCardExtensionService;
using keyple::card::calypso::CalypsoExtensionService;
using keyple::core::service::Plugin;
using keyple::core::service::SmartCardService;
using keyple::core::service::SmartCardServiceProvider;
using keyple::core::util::cpp::Logger;
using keyple::core::util::cpp::LoggerFactory;
using keyple::plugin::pcsc::PcscCardCommunicationProtocol;
using keyple::plugin::pcsc::PcscPluginFactoryBuilder;
using keyple::plugin::pcsc::PcscReader;
using keypop::calypso::card::CalypsoCardApiFactory;
using keypop::calypso::card::card::CalypsoCardSelectionExtension;
using keypop::reader::CardReader;
using keypop::reader::ConfigurableCardReader;
using keypop::reader::ObservableCardReader;
using keypop::reader::ReaderApiFactory;
using keypop::reader::selection::BasicCardSelector;
using keypop::reader::selection::CardSelectionManager;
using keypop::reader::selection::IsoCardSelector;
using keypop::storagecard::MifareClassicKeyType;
using keypop::storagecard::StorageCardApiFactory;
using keypop::storagecard::card::ProductType;
using keypop::storagecard::card::StorageCardSelectionExtension;

/**
 * Main entry point for the multi-technology card detection example using PC/SC
 * and a scheduled selection scenario.
 *
 * <p>This class demonstrates how to configure a PC/SC contactless reader to
 * detect and process multiple card technologies in a single observation loop.
 * The selection scenario is prepared upfront and scheduled on the reader: upon
 * card insertion, it is executed automatically and the observer is notified
 * with the result.
 *
 * <p><b>Supported Technologies:</b>
 *
 * <ul>
 *   <li><b>Calypso cards</b> (ISO 14443-4) – identified by AID; reads the
 *       Environment &amp; Holder file (SFI 07h, record 1) during selection.
 *   <li><b>MIFARE Ultralight</b> – protocol-based selection; reads block 1
 *       during selection.
 *   <li><b>MIFARE Classic 1K</b> – protocol-based selection; authenticates
 *       sector 0 with default Key A then reads block 1 during selection.
 *   <li><b>ST25/SRT512</b> – protocol-based selection; reads block 1 during
 *       selection.
 * </ul>
 *
 * <p><b>Execution Flow:</b>
 *
 * <pre>
 * 1. Initialize PC/SC plugin and reader (activate all supported protocols)
 * 2. Prepare the scheduled multi-technology selection scenario
 * 3. Register a card reader observer and start card detection
 * 4. Wait indefinitely for card insertion events (CTRL-C to exit)
 * 5. On each insertion, the observer logs the card type and the data read
 *    during selection
 * </pre>
 *
 * <p><b>Prerequisites:</b>
 *
 * <ul>
 *   <li>PC/SC compatible contactless reader connected
 *   <li>Official keyple-card-cna-storagecard-java-lib from CNA (for storage
 *       cards)
 *   <li>{@code config.properties} on the classpath with {@code cardReader} and
 *       {@code aid} entries
 * </ul>
 *
 * @author Calypso Networks Association
 * @since 1.0.0
 */
class Main_MultiTech_ScheduledSelection_Pcsc { };
static std::unique_ptr<Logger> logger = LoggerFactory::getLogger(
    typeid(Main_MultiTech_ScheduledSelection_Pcsc));

/**
 * Reader name regex. Edit to match the name of the PC/SC reader actually
 * used to run this example.
 */
static const std::string CARD_READER_NAME_REGEX
    = ".*ASK LoGO.*|.*Contactless.*|.*ACR122U.*|.*00 01.*|.*5x21-CL 0.*";

// =============================================================================
// LOGICAL PROTOCOL IDENTIFIERS
// =============================================================================

/** ISO 14443-4 logical protocol – used for Calypso cards. */
static const std::string ISO_CARD_PROTOCOL = "ISO_14443_4_CARD";

/** MIFARE Ultralight logical protocol. */
static const std::string MIFARE_ULTRALIGHT_PROTOCOL = "MIFARE_ULTRALIGHT";

/** MIFARE Classic logical protocol. */
static const std::string MIFARE_CLASSIC_PROTOCOL = "MIFARE_CLASSIC";

/** ST25/SRT512 logical protocol. */
static const std::string ST25_SRT512_PROTOCOL = "ST25_SRT512";

// =============================================================================
// CALYPSO CONFIGURATION
// =============================================================================

/** AID: Keyple test kit profile 1, Application 2 */
static const std::string AID = "A000000291FF9101";

/**
* Short File Identifier of the Environment &amp; Holder file.
*
* <p>Pre-read during selection to immediately expose cardholder and network
* data.
*/
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;

// =============================================================================
// SERVICE INSTANCES
// =============================================================================

/** PC/SC plugin instance. */
static std::shared_ptr<Plugin> plugin;

/** Configured contactless card reader. */
static std::shared_ptr<CardReader> cardReader;

/**
 * Factory for Calypso-specific objects (selection extension, transaction
 * managers).
 */
static std::shared_ptr<ReaderApiFactory> readerApiFactory;
/*
 * The Calypso factory used to create the selection extension and
 * transaction managers.
 */
static std::shared_ptr<CalypsoCardApiFactory> calypsoCardApiFactory;

// =============================================================================
// INITIALIZATION
// =============================================================================

/**
 * Initializes the Keyple service.
 *
 * <p>Gets an instance of the smart card service, registers the PC/SC
 * plugin, and prepares the reader API factory for use.
 */
static void
initKeypleService()
{
    std::shared_ptr<SmartCardService> smartCardService(
        SmartCardServiceProvider::getService());
    plugin = smartCardService->registerPlugin(
        PcscPluginFactoryBuilder::builder()->build());
    readerApiFactory = smartCardService->getReaderApiFactory();
}

/**
 * Finds, configures, and prepares the contactless PC/SC reader.
 *
 * <p>Activates all four protocol mappings required to detect the supported card
 * technologies:
 *
 * <ul>
 *   <li>ISO 14443-4 → {@value #ISO_CARD_PROTOCOL}
 *   <li>MIFARE Ultralight → {@value #MIFARE_ULTRALIGHT_PROTOCOL}
 *   <li>MIFARE Classic 1K → {@value #MIFARE_CLASSIC_PROTOCOL}
 *   <li>ST25/SRT512 → {@value #ST25_SRT512_PROTOCOL}
 * </ul>
 */
static void
initCardReader()
{
    cardReader = plugin->findReader(CARD_READER_NAME_REGEX);

    std::dynamic_pointer_cast<PcscReader>(
        plugin->getReaderExtension(
            typeid(PcscReader), cardReader->getName()))
        ->setContactless(true)
        .setIsoProtocol(PcscReader::IsoProtocol::T1)
        .setDisconnectionMode(PcscReader::DisconnectionMode::UNPOWER)
        .setSharingMode(PcscReader::SharingMode::SHARED);

    const auto configurableReader
        = std::dynamic_pointer_cast<ConfigurableCardReader>(cardReader);
    configurableReader->activateProtocol(
        PcscCardCommunicationProtocol::ISO_14443_4.getName(),
        ISO_CARD_PROTOCOL);
    configurableReader->activateProtocol(
        PcscCardCommunicationProtocol::MIFARE_ULTRALIGHT.getName(),
        MIFARE_ULTRALIGHT_PROTOCOL);
    configurableReader->activateProtocol(
        PcscCardCommunicationProtocol::MIFARE_CLASSIC_1K.getName(),
        MIFARE_CLASSIC_PROTOCOL);
    configurableReader->activateProtocol(
        PcscCardCommunicationProtocol::ST25_SRT512.getName(),
        ST25_SRT512_PROTOCOL);
}

/**
 * Initializes the Calypso card extension service and retrieves its API
 * factory.
 *
 * <p>The factory is used later to create the CalypsoCardSelectionExtension.
 */
static void
initCalypsoCardExtensionService()
{
    std::shared_ptr<CalypsoExtensionService> calypsoExtensionService(
        CalypsoExtensionService::getInstance());
    SmartCardServiceProvider::getService()->checkCardExtension(
        calypsoExtensionService);
    calypsoCardApiFactory = calypsoExtensionService->getCalypsoCardApiFactory();
}

// =============================================================================
// CARD SELECTION
// =============================================================================

/**
 * Builds the multi-technology scheduled selection scenario.
 *
 * <p>Each selection case targets one card technology. The CardSelectionManager
 * tries them in registration order; the first match wins. Data read during
 * selection is available immediately in the observer callback without an extra
 * card exchange.
 *
 * <p><b>Selection order:</b>
 *
 * <ol>
 *   <li>Calypso (AID-based, ISO 14443-4) – most specific, checked first
 *   <li>MIFARE Ultralight (protocol-based)
 *   <li>MIFARE Classic 1K (protocol-based, requires sector 0 authentication)
 *   <li>ST25/SRT512 (protocol-based)
 * </ol>
 *
 * @return a configured CardSelectionManager ready to be scheduled on the reader
 */
static std::shared_ptr<CardSelectionManager>
prepareCardSelection()
{
    std::shared_ptr<CardSelectionManager> manager(
        readerApiFactory->createCardSelectionManager());

    std::shared_ptr<StorageCardApiFactory> storageCardApiFactory(
        StorageCardExtensionService::getInstance()->getStorageCardApiFactory());

    /* --- Case 1: Calypso card (AID-based, ISO 14443-4) --- */
    std::shared_ptr<IsoCardSelector> calypsoSelector(
        readerApiFactory->createIsoCardSelector());
    calypsoSelector->filterByCardProtocol(ISO_CARD_PROTOCOL)
        .filterByDfName(AID);
    std::unique_ptr<CalypsoCardSelectionExtension> calypsoExtension(
        calypsoCardApiFactory->createCalypsoCardSelectionExtension());
    calypsoExtension->acceptInvalidatedCard().prepareReadRecord(
        SFI_ENVIRONMENT_AND_HOLDER, 1);
    manager->prepareSelection(calypsoSelector, std::move(calypsoExtension));

    /* --- Case 2: MIFARE Ultralight (protocol-based) --- */
    std::shared_ptr<BasicCardSelector> mifareUltralightSelector(
        readerApiFactory->createBasicCardSelector());
    mifareUltralightSelector->filterByCardProtocol(
        MIFARE_ULTRALIGHT_PROTOCOL);
    std::unique_ptr<StorageCardSelectionExtension>
        storageExtensionMifareUltralight(
            storageCardApiFactory->createStorageCardSelectionExtension(
                ProductType::MIFARE_ULTRALIGHT()));
    storageExtensionMifareUltralight->prepareReadBlocks(1, 1);
    manager->prepareSelection(
        mifareUltralightSelector,
        std::move(storageExtensionMifareUltralight));

    /*
     * --- Case 3: MIFARE Classic 1K (protocol-based) ---
     * Block 1 belongs to sector 0 (blocks 0-3); authenticates sector 0 with
     * default Key A (FF:FF:FF:FF:FF:FF) before reading block 1.
     */
    std::shared_ptr<BasicCardSelector> mifareClassicSelector(
        readerApiFactory->createBasicCardSelector());
    mifareClassicSelector->filterByCardProtocol(MIFARE_CLASSIC_PROTOCOL);
    std::unique_ptr<StorageCardSelectionExtension>
        storageExtensionMifareClassic(
            storageCardApiFactory->createStorageCardSelectionExtension(
                ProductType::MIFARE_CLASSIC_1K()));
    storageExtensionMifareClassic
        ->prepareMifareClassicAuthenticate(
            0,
            MifareClassicKeyType::KEY_A,
            std::vector<uint8_t>({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}))
        .prepareReadBlocks(1, 1);
    manager->prepareSelection(
        mifareClassicSelector, std::move(storageExtensionMifareClassic));

    /* --- Case 4: ST25/SRT512 (protocol-based) --- */
    std::shared_ptr<BasicCardSelector> st25Selector(
        readerApiFactory->createBasicCardSelector());
    st25Selector->filterByCardProtocol(ST25_SRT512_PROTOCOL);
    std::unique_ptr<StorageCardSelectionExtension> storageExtensionSt25(
        storageCardApiFactory->createStorageCardSelectionExtension(
            ProductType::ST25_SRT512()));
    storageExtensionSt25->prepareReadBlocks(1, 1);
    manager->prepareSelection(st25Selector, std::move(storageExtensionSt25));

    return manager;
}

// =============================================================================
// MAIN
// =============================================================================

static int
runExample() {
    Logger::setLoggerLevel(Logger::Level::logInfo);

    logger->info(
        "= Multi-Technology Scheduled Selection Demo (PC/SC) ==========\n");
    logger->info(
        "Supported: Calypso, MIFARE Ultralight, MIFARE Classic 1K, "
        "ST25/SRT512\n");

    /* Step 1 - Initialize services and reader */
    initKeypleService();
    initCardReader();
    initCalypsoCardExtensionService();

    /* Step 2 - Build the multi-technology scheduled selection scenario */
    std::shared_ptr<CardSelectionManager> cardSelectionManager(
        prepareCardSelection());

    /*
     * Step 3 - Schedule the scenario on the observable reader.
     * MATCHED_ONLY: the observer is notified only when at least one
     * selection case succeeds.
     */
    auto observableCardReader
        = std::dynamic_pointer_cast<ObservableCardReader>(cardReader);
    cardSelectionManager->scheduleCardSelectionScenario(
        observableCardReader,
        ObservableCardReader::NotificationMode::MATCHED_ONLY);

    /* Step 4 - Register the observer and start detection in repeating mode */
    auto cardReaderObserver = std::make_shared<CardReaderObserver>(
        observableCardReader, cardSelectionManager);
    observableCardReader->setReaderObservationExceptionHandler(
        cardReaderObserver);
    observableCardReader->addObserver(cardReaderObserver);
    observableCardReader->startCardDetection(
        ObservableCardReader::DetectionMode::REPEATING);

    logger->info("Waiting for a card... (press CTRL-C to exit)\n");

    /*
     * Step 5 - Park the main thread; all processing happens in the observer
     * callbacks. The program will remain active indefinitely until
     * interrupted (CTRL-C to exit).
     */
    while (true) {
    }
}

int
main() {
    try {
        return runExample();

    } catch (const std::exception& e) {
        logger->error("Example terminated on exception: %\n", e.what());
        return 1;
    }
}
