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
/////////////////////////////////////////////////////////////////////////
/// A process that corrects the BaseLine. The parameter "SmoothingWindow" sets the time window (number of
/// bins) used for the moving average filter that is used for the baseline correction. Standard value is 75.
/// The range of signal IDs, to which the process is applied, can be selected with the parameter
/// "SignalsRange". The process can be added to the analysis rml for example like \code
///   <addProcess type="TRestRawBaseLineCorrectionProcess" name="blCorrection" value="ON"
///   verboseLevel="silent">
///       <parameter name="signalsRange" value="(4610,4900)" />
///       <parameter name="smoothingWindow" value="75" />
///   </addProcess>
/// \endcode
/// \htmlonly <style>div.image
/// img[src="doc_TRestRawBaseLineCorrectionProcess1.png"]{width:750px;}</style>\endhtmlonly
///
/// ![A raw signal with and without TRestBaseLineCorrectionProcess with smoothing window size
/// 75](doc_TRestRawBaseLineCorrectionProcess1.png)
///
/// \htmlonly <style>div.image
/// img[src="doc_TRestRawBaseLineCorrectionProcess2.png"]{width:750px;}</style>\endhtmlonly
///
/// ![A raw signal with and without TRestBaseLineCorrectionProcess with smoothing window size
/// 75](doc_TRestRawBaseLineCorrectionProcess2.png) <hr>
///
/// \warning **⚠ REST is under continuous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this code
/// up to date. Your feedback will be worth to support this software, please report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2022-Mar:  First implementation
///             Konrad Altenmueller
/// \class TRestRawBaseLineCorrectionProcess
/// \author     Konrad Altenmueller
///
/// <hr>
/////////////////////////////////////////////////////////////////////////

#include "TRestRawBaseLineCorrectionProcess.h"

using namespace std;

ClassImp(TRestRawBaseLineCorrectionProcess);

TRestRawBaseLineCorrectionProcess::TRestRawBaseLineCorrectionProcess() { Initialize(); }

TRestRawBaseLineCorrectionProcess::~TRestRawBaseLineCorrectionProcess() {
    delete fInputEvent;
    delete fOutputEvent;
}

void TRestRawBaseLineCorrectionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputEvent = nullptr;
    fOutputEvent = new TRestRawSignalEvent();
}

TRestEvent* TRestRawBaseLineCorrectionProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fInputEvent->InitializeReferences(run);
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelTypes.empty()) {
        cerr << "TRestRawBaseLineCorrectionProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    for (int s = 0; s < fInputEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* signal = fInputEvent->GetSignal(s);
        const UShort_t signalId = signal->GetSignalID();

        const string channelType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);
        const string channelName = fReadoutMetadata->GetNameForChannelDaqId(signalId);

        // Check if channel type is in the list of selected channel types
        if (!fChannelTypes.empty() && fChannelTypes.find(channelType) == fChannelTypes.end()) {
            // If channel type is not in the selected types, add the signal without baseline correction
            fOutputEvent->AddSignal(*signal);
            continue;
        }

        if (fRangeEnabled && (signal->GetID() < fSignalsRange.X() || signal->GetID() > fSignalsRange.Y())) {
            // If signal is outside the specified range, add the signal without baseline correction
            fOutputEvent->AddSignal(*signal);
            continue;
        }

        TRestRawSignal signalCorrected;
        signal->GetBaseLineCorrected(&signalCorrected, fSmoothingWindow);
        signalCorrected.SetID(signal->GetID());
        fOutputEvent->AddSignal(signalCorrected);
    }

    return fOutputEvent;
}

void TRestRawBaseLineCorrectionProcess::InitProcess() {}

void TRestRawBaseLineCorrectionProcess::InitFromConfigFile() {
    if (fSignalsRange.X() != -1 && fSignalsRange.Y() != -1) {
        fRangeEnabled = true;
    }

    const auto filterType = GetParameter("channelType", "");
    if (!filterType.empty()) {
        fChannelTypes.insert(filterType);
    }

    fSignalsRange = Get2DVectorParameterWithUnits("signalsRange", fSignalsRange);
    fSmoothingWindow = (Int_t)StringToDouble(GetParameter("smoothingWindow", double(fSmoothingWindow)));
}

void TRestRawBaseLineCorrectionProcess::PrintMetadata() {
    BeginPrintProcess();

    if (fChannelTypes.empty()) {
        RESTMetadata << "No type specified. All signal types will be processed." << RESTendl;
    } else {
        RESTMetadata << "Channel type for baseline correction: ";
        for (const auto& channelType : fChannelTypes) {
            RESTMetadata << channelType << " ";
        }
        RESTMetadata << RESTendl;
    }

    RESTMetadata << "Smoothing window size: " << fSmoothingWindow << RESTendl;
    RESTMetadata << "Baseline correction applied to signals with IDs in range (" << fSignalsRange.X() << ","
                 << fSignalsRange.Y() << ")" << RESTendl;

    EndPrintProcess();
}

void TRestRawBaseLineCorrectionProcess::EndProcess() {}
