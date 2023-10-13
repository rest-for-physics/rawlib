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

////////////////////////////////////////////////////////////////////////
/// The TRestRawBiPoSignalAnalysisProcess is meant to add specific BiPo
/// observables when doing the analysis.
/// For the moment it gives the observable "t1t2" which is the time distance
/// between the frist and second window in the BiPo analysis
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2023-Oct: First implementation
///           Ana Quintana
///
/// \class      TRestRawBiPoSignalAnalysisProcess
/// \author     Ana Quintana
///
/// <hr>
///

#include "TRestRawBiPoAnalysisProcess.h"

using namespace std;

ClassImp(TRestRawBiPoAnalysisProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawBiPoAnalysisProcess::TRestRawBiPoAnalysisProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawBiPoAnalysisProcess::~TRestRawBiPoAnalysisProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestRawBiPoAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
    fAnaEvent = NULL;

    // Initialize here the values of class data members if needed
}

///////////////////////////////////////////////
/// \brief Process initialization. Observable names can be re-interpreted
/// here. Any action in the process required before starting event process
/// might be added here.
///
void TRestRawBiPoAnalysisProcess::InitProcess() {
    TRestRawToSignalProcess::InitProcess();

    fEventCounter = 0;
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawBiPoAnalysisProcess::ProcessEvent(TRestEvent* evInput) {
    fSignalEvent = (TRestRawSignalEvent*)evInput;

    // Write here the main logic of process: TRestRawBiPoAnalysisProcess
    // Read data from input event, write data to output event, and save observables to tree

    Double_t t1t2_BiPo = fSignalEvvent->GetAuxiliar();
    SetObservableValue("t1t2", t1t2_BiPo);

    // return fAnaEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestRawBiPoAnalysisProcess::EndProcess() {
    // Write here the jobs to do when all the events are processed
}
