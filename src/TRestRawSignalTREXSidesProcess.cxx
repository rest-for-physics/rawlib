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
///_______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2022-September: First implementation of raw signal TREX-DM detector side 
///                 determination process.
///                 Created from TRestRawSignalAnalysisProcess
///
/// \class      TRestRawSignalTREXSidesProcess
/// \author     David Díez Ibáñez
///
/// <hr>
///

#include "TRestRawSignalTREXSidesProcess.h"

using namespace std;

ClassImp(TRestRawSignalTREXSidesProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalTREXSidesProcess::TRestRawSignalTREXSidesProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalTREXSidesProcess::~TRestRawSignalTREXSidesProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalTREXSidesProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fSignalEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawSignalTREXSidesProcess::InitProcess() {
    
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalTREXSidesProcess::ProcessEvent(TRestEvent* evInput) {
    fSignalEvent = (TRestRawSignalEvent*)evInput;
    
    int south = 0, north = 0;
    
    for( int j=0; j<fSignalEvent->GetNumberOfSignals(); j++ ){ //fRawSignalEvent->GetNumberOfSignals()
        TRestRawSignal* singleSignal = fSignalEvent->GetSignal(j);
        
        if(singleSignal->GetID()<fHalfIdRange){RESTDebug << "Signal " << j << " in plane SOUTH" << RESTendl; south = 1;}
        if(singleSignal->GetID()>=fHalfIdRange){RESTDebug << "Signal " << j << " in plane NORTH" << RESTendl; north = 1;}
    }
    
    if(south>0&&north==0){RESTDebug << "SOUTH event" << RESTendl; SetObservableValue("DetectorSide", south-north);} // 1
    if(south==0&&north>0){RESTDebug << "NORTH event" << RESTendl; SetObservableValue("DetectorSide", south-north);} // -1
    if(south>0&&north>0){RESTDebug << "BOTH sides event" << RESTendl; SetObservableValue("DetectorSide", south+north);} // 2
    if(south==0&&north==0){RESTDebug << "Empty event" << RESTendl; SetObservableValue("DetectorSide", south-north);} // 0
    
    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    return fSignalEvent;
}