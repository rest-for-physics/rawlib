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
/// Process to identify events from North and South side of TREX-DM experiment.
/// Two FEC-Feminos per side, with 4 AGET chips each board.
/// AGET ID assigned following the pattern: 72*8*2, 72 IDs per chip, 8 chips per side,
/// 2 sides.
/// With this configuration, AGET IDs go form 0 to 1151.
/// South side: 0 to 575
/// North side: 576 to 1151
///
/// Metadata parameters that can be defined in the rml:
/// * **southIDs**: Range of AGET IDs for South detector.
/// * **northIDs**: Range of AGET IDs for North detector.
///
/// Example in rml file:
/// \code
/// <addProcess type="TRestRawSignalTREXSidesProcess" name="TREXsides" value="ON"
///   southIDs="(0,575)"
///   northIDs="(576,1151)"
///   observable="all" >
/// </addProcess>
/// \endcode
///
///  ### Observables
///
/// * **DetectorSide**: Different value for North or South or Both sides events.
///     1: South side
///     -1: North side
///     2: Both sides
///     0: Empty event
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
TRestRawSignalTREXSidesProcess::~TRestRawSignalTREXSidesProcess() = default;

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
void TRestRawSignalTREXSidesProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalTREXSidesProcess::ProcessEvent(TRestEvent* inputEvent) {
    fSignalEvent = (TRestRawSignalEvent*)inputEvent;

    int south = 0, north = 0;

    for (int j = 0; j < fSignalEvent->GetNumberOfSignals(); j++) {  // fRawSignalEvent->GetNumberOfSignals()
        TRestRawSignal* singleSignal = fSignalEvent->GetSignal(j);

        if (singleSignal->GetID() >= fSouthIDs.X() && singleSignal->GetID() <= fSouthIDs.Y()) {
            RESTDebug << "Signal " << j << " in plane SOUTH" << RESTendl;
            south = 1;
        }
        if (singleSignal->GetID() >= fNorthIDs.X() && singleSignal->GetID() <= fNorthIDs.Y()) {
            RESTDebug << "Signal " << j << " in plane NORTH" << RESTendl;
            north = 1;
        }
    }

    if (south > 0 && north == 0) {
        RESTDebug << "SOUTH event" << RESTendl;
        SetObservableValue("DetectorSide", south - north);
    }  // 1
    if (south == 0 && north > 0) {
        RESTDebug << "NORTH event" << RESTendl;
        SetObservableValue("DetectorSide", south - north);
    }  // -1
    if (south > 0 && north > 0) {
        RESTDebug << "BOTH sides event" << RESTendl;
        SetObservableValue("DetectorSide", south + north);
    }  // 2
    if (south == 0 && north == 0) {
        RESTDebug << "Empty event" << RESTendl;
        SetObservableValue("DetectorSide", south - north);
    }  // 0

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) {
        return nullptr;
    }

    return fSignalEvent;
}
