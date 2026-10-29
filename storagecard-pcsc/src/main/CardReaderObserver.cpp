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

#include "CardReaderObserver.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include "keyple/core/util/HexUtil.hpp"
#include "keypop/calypso/card/card/CalypsoCard.hpp"
#include "keypop/storagecard/card/ProductType.hpp"
#include "keypop/storagecard/card/StorageCard.hpp"

using keyple::core::util::HexUtil;
using keypop::calypso::card::card::CalypsoCard;
using keypop::storagecard::card::ProductType;
using keypop::storagecard::card::StorageCard;

/* File identifiers */
static const std::uint8_t SFI_ENVIRONMENT_AND_HOLDER = 0x07;

/* ProductType is a plain value type with no built-in name/label. */
static std::string
productTypeToString(const ProductType& productType) {
    if (productType == ProductType::MIFARE_ULTRALIGHT()) {
        return "MIFARE_ULTRALIGHT";
    } else if (productType == ProductType::MIFARE_CLASSIC_1K()) {
        return "MIFARE_CLASSIC_1K";
    } else if (productType == ProductType::MIFARE_CLASSIC_4K()) {
        return "MIFARE_CLASSIC_4K";
    } else if (productType == ProductType::ST25_SRT512()) {
        return "ST25_SRT512";
    } else {
        return "UNKNOWN";
    }
}

CardReaderObserver::CardReaderObserver(
    std::shared_ptr<ObservableCardReader> reader,
    std::shared_ptr<CardSelectionManager> cardSelectionManager)
: mReader(reader)
, mCardSelectionManager(cardSelectionManager) {
}

void
CardReaderObserver::onReaderEvent(
    const std::shared_ptr<CardReaderEvent> event) {
    switch (event->getType()) {
    case CardReaderEvent::CARD_MATCHED: {
        const auto smartCard
            = mCardSelectionManager
                  ->parseScheduledCardSelectionsResponse(
                      event->getScheduledCardSelectionsResponse())
                  ->getActiveSmartCard();

        const auto calypsoCard
            = std::dynamic_pointer_cast<CalypsoCard>(smartCard);
        const auto storageCard
            = std::dynamic_pointer_cast<StorageCard>(smartCard);

        if (calypsoCard != nullptr) {
            mLogger->info(
                "Observer notification: Calypso card selected = %\n",
                calypsoCard);
            mLogger->info(
                "Calypso Serial Number = %\n",
                HexUtil::toHex(calypsoCard->getApplicationSerialNumber()));
            mLogger->info("Data read during the scheduled selection process:\n");
            mLogger->info(
                "File %h, rec 1: FILE_CONTENT = %\n",
                HexUtil::toHex(SFI_ENVIRONMENT_AND_HOLDER),
                calypsoCard->getFileBySfi(SFI_ENVIRONMENT_AND_HOLDER));
        } else if (storageCard != nullptr) {
            mLogger->info(
                "Observer notification: Storage card selected, product = "
                "%\n",
                productTypeToString(storageCard->getProductType()));
            const std::vector<uint8_t> block1 = storageCard->getBlock(1);
            mLogger->info(
                "Block 1 content: %\n", HexUtil::toHex(block1));
        }

        mLogger->info("= #### End of the card processing.\n");
    } break;
    case CardReaderEvent::CARD_INSERTED:
        mLogger->error(
            "CARD_INSERTED event: should not have occurred because of the "
            "MATCHED_ONLY selection mode chosen.\n");
        break;
    case CardReaderEvent::CARD_REMOVED:
        mLogger->trace(
            "There is no card inserted anymore. Return to the waiting "
            "state...\n");
        break;
    default:
        break;
    }

    if (event->getType() == CardReaderEvent::CARD_INSERTED
        || event->getType() == CardReaderEvent::CARD_MATCHED) {
        /*
         * Informs the underlying layer of the end of the card processing, in
         * order to manage the removal sequence.
         */
        mReader->finalizeCardProcessing();
    }
}

void
CardReaderObserver::onReaderObservationError(
    const std::string& pluginName,
    const std::string& readerName,
    const std::shared_ptr<std::exception> e) {
    mLogger->error(
        "An exception occurred in plugin '%', reader '%': %\n",
        pluginName,
        readerName,
        e ? e->what() : "unknown");
}
