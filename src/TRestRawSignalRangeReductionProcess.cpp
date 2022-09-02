/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

#include "TRestRawSignalRangeReductionProcess.h"

using namespace std;

ClassImp(TRestRawSignalRangeReductionProcess);

TRestRawSignalRangeReductionProcess::Initialize() {
    fInputSignalEvent = nullptr;
    fOutputRawSignalEvent = new TRestRawSignalEvent();
}

TRestRawSignalRangeReductionProcess::TRestRawSignalRangeReductionProcess() { Initialize(); }

TRestRawSignalRangeReductionProcess::TRestRawSignalRangeReductionProcess(const char* configFilename) {
    Initialize();
    if (LoadConfigFromFile(configFilename) == -1) {
        LoadDefaultConfig();
    }
}

TRestRawSignalRangeReductionProcess::~TRestRawSignalRangeReductionProcess() { delete fOutputSignalEvent; }

void TRestRawSignalRangeReductionProcess::LoadDefaultConfig() {
    SetName("rawSignalRangeReductionProcess-default");
    SetTitle("Default config");
}

void TRestRawSignalRangeReductionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputSignalEvent = nullptr;
    fOutputSignalEvent = new TRestRawSignalEvent();
}

void TRestRawSignalRangeReductionProcess::LoadConfig(const string& configFilename, const string& name) {
    LoadConfigFromFile(configFilename, name);
}

void TRestRawSignalRangeReductionProcess::InitFromConfigFile() {
    const resolutionInBits = GetParameter("resolutionInBits", fResolutionInBits);
    SetResolutionInBits(resolutionInBits);

    const DigitizationRange = GetParameter("inputRange", fDigitizationInputRange);
    SetDigitizationInputRange(DigitizationRange);
}

void TRestRawSignalRangeReductionProcess::InitProcess() { PrintMetadata(); }

TRestEvent* TRestRawSignalRangeReductionProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputSignalEvent = (TRestRawSignalEvent*)inputEvent;

    if (fInputSignalEvent->GetNumberOfSignals() <= 0) {
        return nullptr;
    }

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal signal = fInputSignalEvent->GetSignal(n);

        fOutputSignalEvent->AddSignal(signal);
    }

    return fOutputSignalEvent;
}

void TRestRawSignalRangeReductionProcess::SetResolutionInNumberOfBits(UShort_t nBits) {
    if (nBits < 0 || nBits > 16) {
        RESTWarning << "Number of bits must be between 1 and 16. Setting it to " << fResolutionInBits
                    << " bits" << RESTendl;
        return;
    }
    fResolutionInBits = nBits;
}

void TRestRawSignalRangeReductionProcess::SetDigitizationRange(const TVector2& range) {
    fDigitizationInputRange = range;

    const limitMin = std::numeric_limits<Short_t>::min();
    const limitMax = std::numeric_limits<Short_t>::max();
    if (range.X() < limitMin) {
        RESTWarning << "TRestRawSignalRangeReductionProcess::SetDigitizationRange - user set start of "
                       "Digitization range to "
                    << range.X() << " which is below the limit of " << limitMin << ". Setting range start to "
                    << limitMin << RESTendl;
        fDigitizationInputRange = TVector2(limitMin, range.Y());
    }
    if (range.Y() > limitMax) {
        RESTWarning << "TRestRawSignalRangeReductionProcess::SetDigitizationRange - user set end of "
                       "Digitization range to "
                    << range.Y() << " which is above the limit of " << limitMax << ". Setting end start to "
                    << limitMax << RESTendl;
        fDigitizationInputRange = TVector2(range.X(), limitMax);
    }
}
