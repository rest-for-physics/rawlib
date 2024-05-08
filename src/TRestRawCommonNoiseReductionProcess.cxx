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

//////////////////////////////////////////////////////////////////////////
///
/// BASIC DOCUMENTATION : Full documentation to be added. A figure with the
/// the pulses before and after applying the process.
///
/// This process tries to reduce the common noise of TRestRawSignals by two
/// means :
///
/// * **Mode = 0**: For each time bin, all time bins are gathered for all the
/// signals
/// and ranked increasingly. We take the middle bin and substract its values to
/// all the bins corresponding to that time.
///
/// * **Mode = 1**:  The method is exactly the same but we take into account
/// *centerWidth%* of the total number of bins center around the middle. The
/// mean
/// of these bins is used to do the correction.
///
/// Common noise identification in all signals or by blocks
/// Written with TREX-DM architecture in mind (8 blocks with 68 signals each
/// and gap of 4 ID between blocks)
///
/// * **Blocks = 0**: All signals together.
///
/// * **Blocks = 1**: 8 groups of signals, independent common noise reduction
/// process for each group.
///
/// Output signal without base line subtraction.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///_______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2020-July: First implementation of common noise reduction process.
///            Benjamin Manier
///
/// 2020-October: Base line not subtracted.
///            David Diez
///
/// \class      TRestRawCommonNoiseReductionProcess
/// \author     Benjamin Manier
/// \author     David Diez
///
/// <hr>
///
#include "TRestRawCommonNoiseReductionProcess.h"

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

ClassImp(TRestRawCommonNoiseReductionProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawCommonNoiseReductionProcess::TRestRawCommonNoiseReductionProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// The default behaviour is that the config file must be specified with
/// full path, absolute or relative.
///
/// \param configFilename A const char* giving the path to an RML file.
///
TRestRawCommonNoiseReductionProcess::TRestRawCommonNoiseReductionProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename)) {
        LoadDefaultConfig();
    }
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawCommonNoiseReductionProcess::~TRestRawCommonNoiseReductionProcess() {
    delete fInputEvent;
    delete fOutputEvent;
}

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawCommonNoiseReductionProcess::LoadDefaultConfig() { SetTitle("Default config"); }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawCommonNoiseReductionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputEvent = nullptr;
    fOutputEvent = new TRestRawSignalEvent();
}

///////////////////////////////////////////////
/// \brief Function to load the configuration from an external configuration
/// file.
///
/// If no configuration path is defined in TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// \param configFilename A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the
/// corresponding TRestGeant4AnalysisProcess section inside the RML.
///
void TRestRawCommonNoiseReductionProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name)) {
        LoadDefaultConfig();
    }
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawCommonNoiseReductionProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawCommonNoiseReductionProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fInputEvent->InitializeReferences(run);
    }

    if (fInputEvent->GetNumberOfSignals() < fMinSignalsRequired) {
        for (int signal = 0; signal < fInputEvent->GetNumberOfSignals(); signal++) {
            fOutputEvent->AddSignal(*fInputEvent->GetSignal(signal));
        }
        return fOutputEvent;
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelType.empty()) {
        cerr << "TRestRawCommonNoiseReductionProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    vector<TRestRawSignal*> signalsToProcess;
    vector<TRestRawSignal*> signalsToIgnore;

    if (fChannelType.empty()) {
        for (int signal = 0; signal < fInputEvent->GetNumberOfSignals(); signal++) {
            signalsToProcess.push_back(fInputEvent->GetSignal(signal));
        }
    } else {
        for (int signal = 0; signal < fInputEvent->GetNumberOfSignals(); signal++) {
            TRestRawSignal* signalPtr = fInputEvent->GetSignal(signal);
            const UShort_t signalId = signalPtr->GetSignalID();
            const string channelType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);

            if (fChannelType == channelType) {
                signalsToProcess.push_back(signalPtr);
            } else {
                signalsToIgnore.push_back(signalPtr);
            }
        }
    }

    // add signals to ignore to the event
    for (auto signal : signalsToIgnore) {
        fOutputEvent->AddSignal(*signal);
    }

    auto eventToProcess = TRestRawSignalEvent();
    for (auto signal : signalsToProcess) {
        eventToProcess.AddSignal(*signal);
    }

    // Event baseline determination.
    Double_t baseLineMean = 0;
    for (int signal = 0; signal < eventToProcess.GetNumberOfSignals(); signal++) {
        eventToProcess.GetSignal(signal)->CalculateBaseLine(20, 150);
        Double_t baseline = eventToProcess.GetSignal(signal)->GetBaseLine();
        baseLineMean += baseline;
    }
    Double_t Baseline = baseLineMean / eventToProcess.GetNumberOfSignals();

    if (fBlocks == 0) {
        Int_t N = eventToProcess.GetNumberOfSignals();

        // if (GetVerboseLevel() >= REST_Debug) N = 1;
        for (int signal = 0; signal < N; signal++) {
            fOutputEvent->AddSignal(*eventToProcess.GetSignal(signal));
        }

        Int_t nBins = eventToProcess.GetSignal(0)->GetNumberOfPoints();
        vector<Double_t> signalValues(N, 0.0);

        for (Int_t bin = 0; bin < nBins; bin++) {
            for (Int_t signal = 0; signal < N; signal++) {
                signalValues[signal] = fOutputEvent->GetSignal(signal)->GetRawData(bin);
            }

            std::sort(signalValues.begin(), signalValues.end());

            // Sorting the different methods
            Int_t begin = 0, middle = 0, end = 0;
            middle = (Int_t)N / 2;
            Double_t norm = 1.0;

            if (fMode == 0) {
                // We take only the middle one
                begin = (Int_t)((double_t)N / 2.0);
                end = begin;
                norm = 1.;
            } else if (fMode == 1) {
                // We take the average of the TRestDetectorSignals at the Center
                begin = middle - (Int_t)(N * fCenterWidth * 0.01);
                end = middle + (Int_t)(N * fCenterWidth * 0.01);
                norm = (Double_t)end - begin;
            }

            // Calculation of the correction to be made to each TRestRawSignal
            Double_t binCorrection = 0.0;
            for (Int_t i = begin; i <= end; i++) binCorrection += signalValues[i];

            binCorrection = binCorrection / norm;

            // Correction applied.
            for (Int_t signal = 0; signal < N; signal++)
                fOutputEvent->GetSignal(signal)->IncreaseBinBy(bin, Baseline - binCorrection);
        }

        return fOutputEvent;
    } else if (fBlocks == 1) {
        Int_t N = 68;
        Int_t nBlocks = 8;
        Int_t firstID = 578;
        Int_t gap = 4;

        Int_t firstInBlock;
        Int_t nSign;
        Int_t sigID;

        for (int block = 0; block < nBlocks; block++) {
            firstInBlock = firstID + block * (N + gap);
            nSign = 0;
            // if (GetVerboseLevel() >= REST_Debug) N = 1;

            for (Int_t signal = 0; signal < N; signal++) {
                sigID = firstInBlock + signal;
                eventToProcess.GetSignalById(sigID)->CalculateBaseLine(20, 500);
                if (eventToProcess.GetSignalById(sigID)->GetBaseLineSigma() >= 3.3) {
                    // debug << "Baseline1: " <<
                    // eventToProcess.GetSignalById(sigID)->GetBaseLineSigma() <<
                    // endl;
                    fOutputEvent->AddSignal(*eventToProcess.GetSignalById(sigID));
                    nSign++;
                }
            }

            Int_t nBins = eventToProcess.GetSignal(0)->GetNumberOfPoints();
            vector<Double_t> signalValues(nSign, 0.0);

            // debug << "nSign: " << nSign << endl;

            for (Int_t bin = 0; bin < nBins; bin++) {
                int i = 0;
                for (Int_t signal = 0; signal < N; signal++) {
                    sigID = firstInBlock + signal;
                    if (eventToProcess.GetSignalById(sigID)->GetBaseLineSigma() >= 3.3) {
                        // debug << "Baseline2: " <<
                        // eventToProcess.GetSignalById(sigID)->GetBaseLineSigma() <<
                        // endl;
                        // debug << fOutputEvent->GetSignalById(sigID)->GetRawData(bin) <<
                        // endl;
                        signalValues[i] = fOutputEvent->GetSignalById(sigID)->GetRawData(bin);
                        i++;
                    }
                }

                std::sort(signalValues.begin(), signalValues.end());

                // Sorting the different methods
                Int_t begin = 0, middle = 0, end = 0;
                middle = (Int_t)nSign / 2;
                Double_t norm = 1.0;

                if (fMode == 0) {
                    // We take only the middle one
                    begin = (Int_t)((double_t)nSign / 2.0);
                    end = begin;
                    norm = 1.;
                } else if (fMode == 1) {
                    // We take the average of the TRestDetectorSignals at the Center
                    begin = middle - (Int_t)(nSign * fCenterWidth * 0.01);
                    end = middle + (Int_t)(nSign * fCenterWidth * 0.01);
                    norm = (Double_t)end - begin;
                }

                // Calculation of the correction to be made to each TRestRawSignal
                Double_t binCorrection = 0.0;
                for (Int_t i = begin; i <= end; i++) {
                    binCorrection += signalValues[i];
                }

                binCorrection = binCorrection / norm;

                // Correction applied.
                for (Int_t signal = 0; signal < N; signal++) {
                    if (eventToProcess.GetSignalById(firstInBlock + signal)->GetBaseLineSigma() >= 3.3) {
                        fOutputEvent->GetSignalById(firstInBlock + signal)
                            ->IncreaseBinBy(bin, Baseline - binCorrection);
                    }
                }
            }
            for (int signal = 0; signal < N; signal++) {
                if (eventToProcess.GetSignalById(firstInBlock + signal)->GetBaseLineSigma() < 3.3) {
                    fOutputEvent->AddSignal(*eventToProcess.GetSignalById(firstInBlock + signal));
                }
            }
        }
        return fOutputEvent;
    }
    return nullptr;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. This method will write the channels histogram.
///
void TRestRawCommonNoiseReductionProcess::EndProcess() {}
