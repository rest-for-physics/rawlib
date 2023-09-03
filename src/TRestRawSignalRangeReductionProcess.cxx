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

TRestRawSignalRangeReductionProcess::TRestRawSignalRangeReductionProcess() { Initialize(); }

TRestRawSignalRangeReductionProcess::TRestRawSignalRangeReductionProcess(const char* configFilename) {
    Initialize();
    if (LoadConfigFromFile(configFilename) == -1) {
        LoadDefaultConfig();
    }
}

TRestRawSignalRangeReductionProcess::~TRestRawSignalRangeReductionProcess() { delete fOutputRawSignalEvent; }

void TRestRawSignalRangeReductionProcess::LoadDefaultConfig() {
    SetName("rawSignalRangeReductionProcess-default");
    SetTitle("Default config");
}

void TRestRawSignalRangeReductionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputRawSignalEvent = nullptr;
    fOutputRawSignalEvent = new TRestRawSignalEvent();
}

void TRestRawSignalRangeReductionProcess::LoadConfig(const string& configFilename, const string& name) {
    LoadConfigFromFile(configFilename, name);
}

void TRestRawSignalRangeReductionProcess::InitFromConfigFile() {
    const UShort_t resolutionInBits =
        (UShort_t)StringToDouble(GetParameter("resolutionInBits", fResolutionInBits));
    SetResolutionInNumberOfBits(resolutionInBits);

    const TVector2 DigitizationRange = Get2DVectorParameterWithUnits("inputRange", fDigitizationInputRange);
    SetDigitizationInputRange(DigitizationRange);
}

void TRestRawSignalRangeReductionProcess::InitProcess() {
    fDigitizationOutputRange = {0, TMath::Power(2, fResolutionInBits) - 1};

    PrintMetadata();
}

TRestEvent* TRestRawSignalRangeReductionProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputRawSignalEvent = (TRestRawSignalEvent*)inputEvent;

    if (fInputRawSignalEvent->GetNumberOfSignals() <= 0) {
        return nullptr;
    }

    for (int n = 0; n < fInputRawSignalEvent->GetNumberOfSignals(); n++) {
        const TRestRawSignal* inputSignal = fInputRawSignalEvent->GetSignal(n);
        TRestRawSignal signal;
        signal.SetSignalID(inputSignal->GetSignalID());

        for (int i = 0; i < inputSignal->GetNumberOfPoints(); i++) {
            const Double_t value = (Double_t)inputSignal->GetData(i);
            Double_t newValue = ConvertFromStartingRangeToTargetRange(value);
            signal.AddPoint(newValue);
        }

        fOutputRawSignalEvent->AddSignal(signal);
    }

    return fOutputRawSignalEvent;
}

void TRestRawSignalRangeReductionProcess::SetResolutionInNumberOfBits(UShort_t nBits) {
    if (nBits < 0 || nBits > 16) {
        RESTWarning << "Number of bits must be between 1 and 16. Setting it to " << fResolutionInBits
                    << " bits" << RESTendl;
        return;
    }
    fResolutionInBits = nBits;
}

void TRestRawSignalRangeReductionProcess::SetDigitizationInputRange(const TVector2& range) {
    fDigitizationInputRange = range;

    const auto limitMin = std::numeric_limits<Short_t>::min();
    const auto limitMax = std::numeric_limits<Short_t>::max();
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

Double_t TRestRawSignalRangeReductionProcess::ConvertFromStartingRangeToTargetRange(Double_t value) const {
    const Double_t conversionFactor = (fDigitizationOutputRange.Y() - fDigitizationOutputRange.X()) /
                                      (fDigitizationInputRange.Y() - fDigitizationInputRange.X());
    const Double_t newValue =
        round(fDigitizationOutputRange.X() + (value - fDigitizationInputRange.X()) * conversionFactor);

    if (newValue < fDigitizationOutputRange.X()) {
        return fDigitizationOutputRange.X();
    } else if (newValue > fDigitizationOutputRange.Y()) {
        return fDigitizationOutputRange.Y();
    }

    return newValue;
}

Double_t TRestRawSignalRangeReductionProcess::ConvertFromTargetRangeToStartingRange(Double_t value) const {
    const Double_t conversionFactor = (fDigitizationInputRange.Y() - fDigitizationInputRange.X()) /
                                      (fDigitizationOutputRange.Y() - fDigitizationOutputRange.X());
    const Double_t newValue =
        round(fDigitizationInputRange.X() + (value - fDigitizationOutputRange.X()) * conversionFactor);

    if (newValue < fDigitizationInputRange.X()) {
        return fDigitizationInputRange.X();
    }
    if (newValue > fDigitizationInputRange.Y()) {
        return fDigitizationInputRange.Y();
    }

    return newValue;
}
