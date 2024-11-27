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
/// The TRestRawFeminosRootToSignalProcess ...
///
/// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
///
/// \warning This process might be obsolete today. It may need additional
/// revision, validation, and documentation. Use it under your own risk. If you
/// find this process useful for your work feel free to use it, improve it,
/// validate and/or document this process. If all those points are addressed
/// these lines can be removed.
///
/// <hr>
///
/// \warning **⚠ REST is under continuous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find will using it at [The REST Framework
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
/// 2024-Aug: First implementation
///           Luis Obis
///
/// \class      TRestRawFeminosRootToSignalProcess
/// \author     Luis Obis
///
/// <hr>
///

#include "TRestRawFeminosRootToSignalProcess.h"

using namespace std;

ClassImp(TRestRawFeminosRootToSignalProcess);

TRestRawFeminosRootToSignalProcess::TRestRawFeminosRootToSignalProcess() { Initialize(); }

TRestRawFeminosRootToSignalProcess::TRestRawFeminosRootToSignalProcess(const char* configFilename) {
    Initialize();
}

TRestRawFeminosRootToSignalProcess::~TRestRawFeminosRootToSignalProcess() {}

void TRestRawFeminosRootToSignalProcess::Initialize() {
    delete fSignalEvent;
    fSignalEvent = new TRestRawSignalEvent();

    fIsExternal = true;  // We need this in order to prevent error since we are not reading a rest root file
    fSingleThreadOnly = true;
}

void TRestRawFeminosRootToSignalProcess::InitProcess() {
    // print input aqs file
    const auto inputFilename = fRunInfo->GetInputFileName(0);

    // verify it is a root file reading the last 5 characters
    if (inputFilename.substr(inputFilename.size() - 5) != ".root") {
        cerr << "TRestRawFeminosRootToSignalProcess::InitProcess: Input file is not a root file" << endl;
        exit(1);
    }

    fInputFile = TFile::Open(inputFilename.c_str(), "READ");
    if (!fInputFile) {
        cerr << "TRestRawFeminosRootToSignalProcess::InitProcess: Error opening input file" << endl;
        exit(1);
    }
    fInputFile->cd();

    fInputRunTree = fInputFile->Get<TTree>("run");
    fInputEventTree = fInputFile->Get<TTree>("events");

    if (!fInputRunTree || !fInputEventTree) {
        cerr << "TRestRawFeminosRootToSignalProcess::InitProcess: Error opening input trees" << endl;
        exit(1);
    }

    fRunInfo->SetFeminosDaqTotalEvents(fInputEventTree->GetEntries());

    fInputEventTree->SetBranchAddress("timestamp", &fInputEventTreeTimestamp);
    fInputEventTree->SetBranchAddress("signal_ids", &fInputEventTreeSignalIds);
    fInputEventTree->SetBranchAddress("signal_values", &fInputEventTreeSignalValues);
}

TRestEvent* TRestRawFeminosRootToSignalProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent->Initialize();

    fInputEventTree->GetEntry(fInputTreeEntry);

    fSignalEvent->SetID(fInputTreeEntry);

    // fInputEventTreeTimestamp is in milliseconds and TRestEvent::SetTime(seconds, nanoseconds)
    fSignalEvent->SetTime(fInputEventTreeTimestamp / 1000, fInputEventTreeTimestamp % 1000 * 1000000);

    for (size_t i = 0; i < fInputEventTreeSignalIds->size(); i++) {
        auto signal = TRestRawSignal();
        signal.Initialize();

        const auto id = fInputEventTreeSignalIds->at(i);
        signal.SetID(id);

        for (int j = 0; j < 512; j++) {
            signal.AddPoint(short(fInputEventTreeSignalValues->at(i * 512 + j)));
        }

        fSignalEvent->AddSignal(signal);
    }

    fInputTreeEntry += 1;

    if (fInputTreeEntry >= fInputEventTree->GetEntries()) {
        return nullptr;
    }

    return fSignalEvent;
}
