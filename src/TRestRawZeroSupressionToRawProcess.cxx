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
/// TRestRawZeroSuppressionToRaw process remove the offset on a zerosuppression
/// acquired event using the first bin as baseline.
///
/// ### Examples
/// Give examples of usage and RML descriptions that can be tested.
/// \code
///     <addProcess type="TRestRawZeroSupressionToRawProcess" name="zS2Raw" value="ON" />
/// \endcode
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2023-June: First implementation of TRestRawZeroSupressionToRawProcess
/// JuanAn Garcia
///
/// \class TRestRawZeroSupressionToRawProcess
/// \author: JuanAn Garcia juanangp@unizar.es
///
/// <hr>
///

#include "TRestRawZeroSupressionToRawProcess.h"

ClassImp(TRestRawZeroSupressionToRawProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawZeroSupressionToRawProcess::TRestRawZeroSupressionToRawProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawZeroSupressionToRawProcess::~TRestRawZeroSupressionToRawProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestRawZeroSupressionToRawProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
    fEvent = nullptr;
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
void TRestRawZeroSupressionToRawProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawZeroSupressionToRawProcess::ProcessEvent(TRestEvent* evInput) {
    fEvent = (TRestRawSignalEvent*)evInput;
    for (int n = 0; n < fEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal* rawSignal = fEvent->GetSignal(n);
        rawSignal->ZeroSuppressionToRaw();
    }

    return fEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestRawZeroSupressionToRawProcess::EndProcess() {}
