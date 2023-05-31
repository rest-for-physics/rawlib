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
/// The TRestRawTDSToSignalProcess is a process used to read a binary file
/// produced by the read binary files produced with TDS (Tektronix oscilloscope) DAQ,
/// The results are stores following TRestRawSignalEvent format ,and then it
/// processing can continue inside with the REST framework libraries.
///
/// ### Parameters
/// Inherits from TRestRawToSignalProcess, only electronics type is required:
/// * **electronics**: You need to write here TDS
///
/// ### Examples
/// Similar to TRestRawToSignalProcess, we just define the `electronics` parameter as `TDS`
/// \code
///   <addProcess type="TRestRawTDSToSignalProcess" name="tdsDAQ" electronics="TDS"/>
/// \endcode
///
///----------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2022-05: First implementation of TRestRawTDSToSignalProcess
/// JuanAn Garcia
///
/// \class TRestRawTDSToSignalProcess
/// \author: JuanAn Garcia juanangp@unizar.es
///
/// <hr>
///

#include "TRestRawTDSToSignalProcess.h"

ClassImp(TRestRawTDSToSignalProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawTDSToSignalProcess::TRestRawTDSToSignalProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawTDSToSignalProcess::~TRestRawTDSToSignalProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name, calls parent TRestRawToSignalProcess::Initialize()
///
void TRestRawTDSToSignalProcess::Initialize() { TRestRawToSignalProcess::Initialize(); }

///////////////////////////////////////////////
/// \brief Process initialization. Read first header
/// block and initializes several variables such as:
/// nSamples, nChannels, fRate, pulseDepth, fScale and
/// negPolarity. It also sets the start timeStamp for
/// the run
///
void TRestRawTDSToSignalProcess::InitProcess() {
    ANABlockHead blockhead;
    if (fread(&blockhead, sizeof(blockhead), 1, fInputBinFile) != 1) return;
    totalBytesReaded = sizeof(blockhead);
    nSamples = blockhead.NEvents;
    nChannels = blockhead.NHits / blockhead.NEvents;
    fRate = blockhead.SRate;
    pulseDepth = blockhead.PSize;
    RESTDebug << "nSamples " << nSamples << " NChannels " << nChannels << RESTendl;
    RESTDebug << "SRATE: " << (double)fRate << " PULSE DEPTH: " << pulseDepth
              << " PRETRIGGER: " << blockhead.Pretrigger << RESTendl;
    for (int i = 0; i < nChannels; i++) {
        fScale[i] = blockhead.mVdiv[i];
        negPolarity[i] = blockhead.NegPolarity[i];
    }
    tNow = static_cast<double>(blockhead.TimeStamp);
    fRunInfo->SetStartTimeStamp(tNow);
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawTDSToSignalProcess::ProcessEvent(TRestEvent* evInput) {
    // TDS block and event header
    ANABlockHead blockhead;
    ANAEventHead eventhead;

    // Initialize fSignalEvent, so it is empty if already filled in
    fSignalEvent->Initialize();

    // Read block header if any, note that we have nSamples events between 2 block headers
    if (nEvents % nSamples == 0 && nEvents != 0) {
        if (fread(&blockhead, sizeof(blockhead), 1, fInputBinFile) != 1) return nullptr;
        totalBytesReaded += sizeof(blockhead);
        // Update timestamp from the blockHeader
        tNow = static_cast<double>(blockhead.TimeStamp);
    }

    // Always read event header at the beginning of event
    if (fread(&eventhead, sizeof(eventhead), 1, fInputBinFile) != 1) return nullptr;
    totalBytesReaded += sizeof(eventhead);
    // This vector holds data which has a length of pulseDepth
    std::vector<Char_t> buffer(pulseDepth);
    fSignalEvent->SetID(nEvents);
    fSignalEvent->SetTime(tNow + static_cast<double>(eventhead.clockTicksLT) * 1E-6);
    // Need to initialize TRestRawSignal with the proper data length
    TRestRawSignal sgnl(pulseDepth);

    // We loop over the recorded channels, we have one data frame per channel
    for (int i = 0; i < nChannels; i++) {
        sgnl.SetSignalID(i);
        fSignalEvent->AddSignal(sgnl);
        // Read data frame and store in buffer
        if (fread((char*)&buffer[0], pulseDepth, 1, fInputBinFile) != 1) return nullptr;
        totalBytesReaded += pulseDepth;
        for (int j = 0; j < pulseDepth; j++) {
            Short_t data = buffer[j];
            if (negPolarity[i]) data *= -1;  // Inversion in case pulses are negative
            data += 128;                     // Add 128 since the oscilloscope range is [-128:128]
            fSignalEvent->AddChargeToSignal(i, j, data);
        }
    }

    // Set end time stamp for the run
    fRunInfo->SetEndTimeStamp(tNow + static_cast<double>(eventhead.clockTicksLT) * 1E-6);
    nEvents++;

    return fSignalEvent;
}
