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
/// TRestRawSignalAddNoiseProcess is ... a longer description comes here
///
/// DOCUMENTATION TO BE WRITTEN (main description, figures, event before
/// and after, ...)
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
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
/// 2016-February: First concept and implementation of TRestRawSignalAddNoise
/// process.
/// \author     Javier Gracia
///
/// \class TRestRawSignalAddNoiseProcess
///
/// <hr>
///

#include "TRestRawSignalAddNoiseProcess.h"
using namespace std;

#include <TFile.h>

ClassImp(TRestRawSignalAddNoiseProcess);
//______________________________________________________________________________
TRestRawSignalAddNoiseProcess::TRestRawSignalAddNoiseProcess() { Initialize(); }

//______________________________________________________________________________
TRestRawSignalAddNoiseProcess::TRestRawSignalAddNoiseProcess(char* cfgFileName) {
    Initialize();

    if (LoadConfigFromFile(cfgFileName) == -1) LoadDefaultConfig();

    PrintMetadata();
    // TRestRawSignalAddNoiseProcess default constructor
}

//______________________________________________________________________________
TRestRawSignalAddNoiseProcess::~TRestRawSignalAddNoiseProcess() {
    delete fOutputSignalEvent;
    // TRestRawSignalAddNoiseProcess destructor
}

void TRestRawSignalAddNoiseProcess::LoadDefaultConfig() {
    SetName("addSignalNoiseProcess-Default");
    SetTitle("Default config");
}

//______________________________________________________________________________
void TRestRawSignalAddNoiseProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputSignalEvent = NULL;
    fOutputSignalEvent = new TRestRawSignalEvent();
}

void TRestRawSignalAddNoiseProcess::LoadConfig(string cfgFilename, string name) {
    if (LoadConfigFromFile(cfgFilename, name) == -1) LoadDefaultConfig();
}

//______________________________________________________________________________
void TRestRawSignalAddNoiseProcess::InitProcess() {
    // Function to be executed once at the beginning of process
    // (before starting the process of the events)

    // Start by calling the InitProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::InitProcess();
}

//______________________________________________________________________________
TRestEvent* TRestRawSignalAddNoiseProcess::ProcessEvent(TRestEvent* evInput) {
    fInputSignalEvent = (TRestRawSignalEvent*)evInput;

    // cout<<"Number of signals "<< fInputSignalEvent->GetNumberOfSignals()<<
    // endl;

    if (fInputSignalEvent->GetNumberOfSignals() <= 0) return NULL;

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal noiseSignal;

        // Asign ID and add noise
        fInputSignalEvent->GetSignal(n)->GetWhiteNoiseSignal(&noiseSignal, fNoiseLevel);
        noiseSignal.SetSignalID(fInputSignalEvent->GetSignal(n)->GetSignalID());

        fOutputSignalEvent->AddSignal(noiseSignal);
    }

    return fOutputSignalEvent;
}

//______________________________________________________________________________
void TRestRawSignalAddNoiseProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}
