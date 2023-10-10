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
/// For the moment it gives the observable T1-T2_distance
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2023-Oct: First implementation (from https://gitlab.in2p3.fr/bipo/matacqana.git)
///           Ana Quintana
///
/// \class      TRestRawBiPoSignalAnalysisProcess
/// \author     Ana Quintana
///
/// <hr>
///

#include "TRestRawBiPoAnalysisProcess.h"

using namespace std;

ClassImp(TRestRawBiPoSignalAnalysisProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawBiPoSignalAnalysisProcess::TRestRawBiPoToSignalProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawBiPoSignalAnalysisProcess::~TRestRawBiPoToSignalProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawBiPoSignalAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
    fSignalEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization. Data members that require initialization just before start processing
/// should be initialized here.
///
void TRestRawBiPoToSignalProcess::InitProcess() {
    TRestRawToSignalProcess::InitProcess();

    fEventCounter = 0;
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawBiPoSignalAnalysisProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = (TRestRawSignalEvent*)inputEvent;

    Double_t t1t2_BiPo = event.GetAuxiliar();
    SetObservableValue("T1-T2_distance", t1t2_BiPo);
}
