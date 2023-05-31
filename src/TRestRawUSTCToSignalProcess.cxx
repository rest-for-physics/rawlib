﻿/*************************************************************************
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
/// The TRestRawUSTCToSignalProcess ... is NOT documented.
///
/// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
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
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 201X-X:    First implementation
///            SJTU PandaX-III
///
/// \class      TRestRawUSTCToSignalProcess
/// \author     SJTU PandaX-III
///
/// <hr>
///

// int counter = 0;

#include "TRestRawUSTCToSignalProcess.h"

using namespace std;

#include <bitset>

#include "TTimeStamp.h"

ClassImp(TRestRawUSTCToSignalProcess);

TRestRawUSTCToSignalProcess::TRestRawUSTCToSignalProcess() { Initialize(); }

TRestRawUSTCToSignalProcess::TRestRawUSTCToSignalProcess(const char* configFilename) { Initialize(); }

TRestRawUSTCToSignalProcess::~TRestRawUSTCToSignalProcess() {
    // TRestRawUSTCToSignalProcess destructor
}

void TRestRawUSTCToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
}

void TRestRawUSTCToSignalProcess::InitProcess() {
    fEventBuffer.clear();
    errorevents.clear();
    unknownerrors = 0;
    fLastBufferedId = 0;

#ifndef Incoherent_Event_Generation
    nBufferedEvent = StringToInteger(GetParameter("BufferNumber", "2"));
    if (nBufferedEvent < 1) nBufferedEvent = 1;
#else
    nBufferedEvent = 2;
#endif  // !Incoherent_Readout

    for (int n = 0; n < nBufferedEvent + 1; n++) {
        fEventBuffer.push_back(vector<USTCDataFrame>());
    }

    fRunOrigin = fRunInfo->GetRunNumber();
    fCurrentFile = 0;
    fCurrentBuffer = 0;
    totalBytesReaded = 0;

    USTCDataFrame frame;
    if ((!GetNextFrame(frame)) || (!ReadFrameData(frame))) {
        FixToNextFrame(fInputFiles[fCurrentFile]);
        if ((!GetNextFrame(frame)) || (!ReadFrameData(frame))) {
            RESTError << "TRestRawUSTCToSignalProcess: Failed to read the first data "
                         "frame in file, may be wrong "
                         "input?"
                      << RESTendl;
            exit(1);
        }
    }

    fCurrentEvent = frame.evId;
    AddBuffer(frame);

    if (fCurrentEvent != 0) {
        RESTWarning << "TRestRawUSTCToSignalProcess : first event is not with id 0 !" << RESTendl;
        RESTWarning << "The first Id is " << fCurrentEvent << ". May be input file not the first file?"
                    << RESTendl;
    }
}

TRestEvent* TRestRawUSTCToSignalProcess::ProcessEvent(TRestEvent* inputEvent) {
    while (1) {
        if (EndReading()) {
            return nullptr;
        }
        if (!FillBuffer()) {
            fSignalEvent->SetOK(false);
        }
        if (fEventBuffer[fCurrentBuffer].size() == 0) {
            RESTDebug << "Blank event " << fCurrentEvent << " !" << RESTendl;
            fCurrentEvent++;
            ClearBuffer();
        } else {
            break;
        }
    }

    fSignalEvent->Initialize();
    fSignalEvent->SetID(fCurrentEvent);

    RESTDebug << "------------------------------------" << RESTendl;
    RESTDebug << "Generating event with ID: " << fCurrentEvent << RESTendl;

    // some event level operation
    USTCDataFrame* frame0 = &fEventBuffer[fCurrentBuffer][0];
    TTimeStamp tSt = 0;
    Long64_t evtTime = frame0->eventTime;
    tSt.SetNanoSec((fTimeOffset + evtTime) % ((Long64_t)1e9));
    tSt.SetSec((fTimeOffset + evtTime) / ((Long64_t)1e9));

    // some signal level operation
    for (unsigned int i = 0; i < fEventBuffer[fCurrentBuffer].size(); i++) {
        USTCDataFrame* frame = &fEventBuffer[fCurrentBuffer][i];
        if (frame->evId == fCurrentEvent && frame->eventTime == evtTime) {
            sgnl.Initialize();
            sgnl.SetSignalID(frame->signalId);
            for (int j = 0; j < 512; j++) {
                sgnl.AddPoint((Short_t)frame->dataPoint[j]);
            }
            fSignalEvent->AddSignal(sgnl);

            RESTDebug << "AsAdId, AgetId, chnId, max value: " << frame->boardId << ", " << frame->chipId
                      << ", " << frame->channelId << ", " << sgnl.GetMaxValue() << RESTendl;

        } else {
            RESTWarning << "TRestRawUSTCToSignalProcess : unmatched signal frame!" << RESTendl;
            RESTWarning << "ID (supposed, received): " << fCurrentEvent << ", " << frame->evId << RESTendl;
            RESTWarning << "Time (supposed, received) : " << evtTime << ", " << frame->eventTime << RESTendl;
            RESTWarning << RESTendl;
            fSignalEvent->SetOK(false);
            fCurrentEvent++;
            ClearBuffer();
            return fSignalEvent;
        }
    }

    ClearBuffer();

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) {
        cout << "------------------------------------" << endl;
        GetChar();
    }
    fSignalEvent->SetTimeStamp(tSt);
    fSignalEvent->SetRunOrigin(fRunOrigin);
    fSignalEvent->SetSubRunOrigin(fSubRunOrigin);

    // cout << fSignalEvent->GetNumberOfSignals() << endl;
    // if( fSignalEvent->GetNumberOfSignals( ) == 0 ) return nullptr;
    fCurrentEvent++;

    return fSignalEvent;
}

void TRestRawUSTCToSignalProcess::EndProcess() {
    for (unsigned int i = 0; i < errorevents.size(); i++) {
        RESTWarning << "Event " << errorevents[i] << " contains error !" << RESTendl;
    }
    if (errorevents.size() > 0 && unknownerrors > 0) {
        RESTWarning << "There are also " << unknownerrors << " errors from unknown events! " << RESTendl;
    } else if (unknownerrors > 0) {
        RESTWarning << "There are " << unknownerrors << " errors from unknown events! " << RESTendl;
    }

    errorevents.clear();
}

bool TRestRawUSTCToSignalProcess::FillBuffer() {
#ifdef Incoherent_Event_Generation
    bool unknowncurrentevent = false;
    if (fEventBuffer[fCurrentBuffer].size() > 0) {
        fCurrentEvent = fEventBuffer[fCurrentBuffer][0].evId;
    } else {
        unknowncurrentevent = true;
    }

    while (1)
#else
    while (fLastBufferedId < fCurrentEvent + ((int)fEventBuffer.size() - 1) / 2)
#endif
    {
        bool errortag = false;
        bool breaktag = false;
        USTCDataFrame frame;
        if (!GetNextFrame(frame)) {
            break;
        }
        if (!ReadFrameData(frame)) {
            RESTWarning << "error reading frame data in file " << fCurrentFile << RESTendl;
            FixToNextFrame(fInputFiles[fCurrentFile]);
            GetNextFrame(frame);
            ReadFrameData(frame);
            errortag = true;
        }
#ifdef Incoherent_Event_Generation
        if (unknowncurrentevent) {
            cout << frame.evId << endl;
            fCurrentEvent = frame.evId;
            unknowncurrentevent = false;
        }

        if (frame.evId != fCurrentEvent) {
            breaktag = true;
        }
#else
        if (frame.evId >= fCurrentEvent + ((int)fEventBuffer.size() - 1) / 2) {
            breaktag = true;
        }
#endif  // Incoherent_Event_Generation

        if (!AddBuffer(frame)) {
            errortag = true;
        }

        if (errortag) {
            if (frame.evId != -1) {
                if (errorevents.size() == 0) {
                    errorevents.push_back(frame.evId);
                } else {
                    for (unsigned int i = 0; i < errorevents.size(); i++) {
                        if (errorevents[i] == frame.evId) {
                            break;
                        } else if (i == errorevents.size() - 1) {
                            errorevents.push_back(frame.evId);
                            break;
                        }
                    }
                }

            } else {
                unknownerrors++;
            }
        }

        if (breaktag) {
            fLastBufferedId = frame.evId;
            break;
        }
    }
    for (unsigned int i = 0; i < errorevents.size(); i++) {
        if (errorevents[i] == fCurrentEvent) return false;
    }
    return true;
}

bool TRestRawUSTCToSignalProcess::OpenNextFile(USTCDataFrame& frame) {
    if (fCurrentFile < (int)fInputFiles.size() - 1)  // try to get frame form next file
    {
        fCurrentFile++;
        return GetNextFrame(frame);
    } else {
        return false;
    }
}

bool TRestRawUSTCToSignalProcess::GetNextFrame(USTCDataFrame& frame) {
    if (fInputFiles[fCurrentFile] == nullptr) {
        return OpenNextFile(frame);
    }
#ifdef V4_Readout_Format
    while (1) {
        UChar_t Protocol[PROTOCOL_SIZE];
        if (fread(Protocol, PROTOCOL_SIZE, 1, fInputFiles[fCurrentFile]) != 1 ||
            feof(fInputFiles[fCurrentFile])) {
            fclose(fInputFiles[fCurrentFile]);
            fInputFiles[fCurrentFile] = nullptr;
            return OpenNextFile(frame);
        }
        totalBytesReaded += PROTOCOL_SIZE;

        if (!(Protocol[0] ^ 0xac) && !(Protocol[1] ^ 0x0f)) {
            // the first 2 bytes must be 0xac0f, otherwise it is wrong

            int flag = Protocol[2] >> 5;
            if (flag & 0x1) {
                // this is the evt_ending frame
                memcpy(fEnding, Protocol, PROTOCOL_SIZE);
                if (fread(fEnding + PROTOCOL_SIZE, ENDING_SIZE - PROTOCOL_SIZE, 1,
                          fInputFiles[fCurrentFile]) != 1 ||
                    feof(fInputFiles[fCurrentFile])) {
                    fclose(fInputFiles[fCurrentFile]);
                    fInputFiles[fCurrentFile] = nullptr;
                    return OpenNextFile(frame);
                }
                totalBytesReaded += ENDING_SIZE;
            } else if (flag & 0x2) {
                // this is the evt_header frame
                memcpy(fHeader, Protocol, PROTOCOL_SIZE);
                if (fread(fHeader + PROTOCOL_SIZE, HEADER_SIZE - PROTOCOL_SIZE, 1,
                          fInputFiles[fCurrentFile]) != 1 ||
                    feof(fInputFiles[fCurrentFile])) {
                    fclose(fInputFiles[fCurrentFile]);
                    fInputFiles[fCurrentFile] = nullptr;
                    return OpenNextFile(frame);
                }
                totalBytesReaded += HEADER_SIZE;
            } else {
                // this is the evt_data frame
                memcpy(frame.data, Protocol, PROTOCOL_SIZE);
                if (fread(frame.data + PROTOCOL_SIZE, DATA_SIZE - PROTOCOL_SIZE, 1,
                          fInputFiles[fCurrentFile]) != 1 ||
                    feof(fInputFiles[fCurrentFile])) {
                    fclose(fInputFiles[fCurrentFile]);
                    fInputFiles[fCurrentFile] = nullptr;
                    return OpenNextFile(frame);
                }
                totalBytesReaded += DATA_SIZE;
                return true;
            }
        } else {
            return false;
        }
    }
#else
    if (fread(frame.data, DATA_SIZE, 1, fInputFiles[fCurrentFile]) != 1 || feof(fInputFiles[fCurrentFile])) {
        fclose(fInputFiles[fCurrentFile]);
        fInputFiles[fCurrentFile] = nullptr;
        return OpenNextFile(frame);
    }
    totalBytesReaded += DATA_SIZE;

    if (frame.data[0] * 0x100 + frame.data[1] != 0xEEEE) {
        RESTarning << "wrong header!" << RESTendl;
        return false;
    }
#endif  // V4_Readout_Format

    return true;
}

// it find the next flag of frame, e.g. 0xffff or 0xac0f
void TRestRawUSTCToSignalProcess::FixToNextFrame(FILE* f) {
    if (f == nullptr) return;
    UChar_t buffer[PROTOCOL_SIZE];
    int n = 0;
    while (1) {
        if (fread(buffer, PROTOCOL_SIZE, 1, f) != 1 || feof(f)) {
            return;
        }
        n += PROTOCOL_SIZE;
#ifdef V4_Readout_Format
        if (!(buffer[0] ^ 0xac) && !(buffer[1] ^ 0x0f)) {
            int flag = buffer[2] >> 5;
            if (flag & 0x2) {
                // we have meet the next event header
                memcpy(fHeader, buffer, PROTOCOL_SIZE);
                if (fread(fHeader + PROTOCOL_SIZE, HEADER_SIZE - PROTOCOL_SIZE, 1,
                          fInputFiles[fCurrentFile]) != 1 ||
                    feof(fInputFiles[fCurrentFile])) {
                    fclose(f);
                    f = nullptr;
                    break;
                }
                n += HEADER_SIZE;
                RESTWarning << "successfully switched to next frame ( + " << n << " byte)" << RESTendl;
                RESTWarning << RESTendl;
                break;
            }
        }
#else
        if (!(buffer[0] ^ 0xff) && !(buffer[1] ^ 0xff) && !(buffer[2] ^ 0xff) && !(buffer[3] ^ 0xff)) {
            RESTWarning << "successfully switched to next frame ( + " << n << " byte)" << RESTendl;
            RESTWarning << RESTendl;
            break;
        }
#endif
    }
    totalBytesReaded += n;
}

bool TRestRawUSTCToSignalProcess::ReadFrameData(USTCDataFrame& frame) {
#ifdef V3_Readout_Format_Long

    // EEEE | E0A0 | 246C 0686 4550 504E | 0001 | 2233 4455 6677 | (A098)(A09C)...
    // | FFFF FFFF
    // 0~1header | 2~3board number | 4~11event time | 12~13channel id(0~63)
    // | 14~19event id | [chip id + data(0~4095)]*512 | ending
    frame.boardId = frame.data[2] & 0x0F;
    frame.chipId = (frame.data[3] & 0xF0) / 16 - 10;
    frame.readoutType = frame.data[3] & 0x0F;
    Long64_t tmp = (Long64_t)frame.data[5] * 0x10000 + (Long64_t)frame.data[6] * 0x100 +
                   (Long64_t)frame.data[7];  // we omit the first byte in case the number is too large
    frame.eventTime = tmp * 0x100000000 + (Long64_t)frame.data[8] * 0x1000000 +
                      (Long64_t)frame.data[9] * 0x10000 + (Long64_t)frame.data[10] * 0x100 +
                      (Long64_t)frame.data[11];
    frame.channelId = frame.data[12] * 0x100 + frame.data[13];
    frame.evId = (frame.data[16] & 0x7F) * 0x1000000 + frame.data[17] * 0x10000 + frame.data[18] * 0x100 +
                 frame.data[19];  // we omit the first 17 bits in case the number
                                  // is too large

    frame.signalId = frame.boardId * 4 * 64 + frame.chipId * 64 + frame.channelId;
#endif

#ifdef V3_Readout_Format_Short
    // EEEE | E0A0 | 246C 0686 | 0001 | 2233 | (A098)(A09C)... | FFFF
    // 0~1header | 2~3board number | 4~7event time | 8~9channel id(0~63) |
    // 10~11event id | [chip id + data(0~4095)]*512 | ending
    frame.boardId = frame.data[2] & 0x0F;
    frame.chipId = (frame.data[3] & 0xF0) / 16 - 10;
    frame.readoutType = frame.data[3] & 0x0F;
    Long64_t tmp = (Long64_t)frame.data[4] * 0x1000000 + (Long64_t)frame.data[5] * 0x10000 +
                   (Long64_t)frame.data[6] * 0x100 + (Long64_t)frame.data[7];
    frame.eventTime = tmp;
    frame.channelId = frame.data[8] * 0x100 + frame.data[9];
    frame.evId = frame.data[10] * 256 + frame.data[11];

    frame.signalId = frame.boardId * 4 * 64 + frame.chipId * 64 + frame.channelId;

#endif  // Long_Readout_Format

#ifdef V4_Readout_Format

    // the evt header frame
    // AC0F        | 401C         | 0300                                     |
    // 010A 3140 0000            |
    // 0500 0000               | .... | .... | 8E95 B452 0~1Protocol | 2~3
    // 010+size | 4~5:
    // 00000011+ETYPE(2)+ST(1)+SOURCEID(5) | 6~11 time stamp(inverted) | 12~15
    // event id(inverted) | not used
    // | ending
    frame.readoutType = fHeader[5] & 0xc0;

    int t_high = fHeader[10] * 0x100 + fHeader[11];
    int t_mid = fHeader[8] * 0x100 + fHeader[9];
    int t_low = fHeader[6] * 0x100 + fHeader[7];
    Long64_t tmp = (Long64_t)t_high * 0x100000000 + (Long64_t)t_mid * 0x10000 + (Long64_t)t_low;
    frame.eventTime = tmp;

    int id_high = fHeader[14] * 0x100 + fHeader[15];
    int id_low = fHeader[12] * 0x100 + fHeader[13];
    frame.evId = id_high * 0x10000 + id_low;

    // the signal frame
    // AC0F  | 0404   | C000  | (3163)(316C)...    | 0000 BCEB 5742
    // 0~1Protocol | 2~3 not used | 4~5: 11+card(5)+chip(2)+channel(7) |
    // [0011+data(0~4095)]*512 | ending
    // event info(time, id, etc.) is in event header
    frame.boardId = (frame.data[4] & 0x3e) >> 1;
    frame.chipId = (frame.data[4] & 0x01) * 2 + (frame.data[5] >> 7);
    frame.channelId = frame.data[5] & 0x7f;

    frame.signalId = frame.boardId * 4 * 68 + frame.chipId * 68 + frame.channelId;

    fChannelOffset.insert(frame.boardId * 4 * 68 + frame.chipId * 68);
#endif

    // sampling point data
    for (int i = 0; i < 512; i++) {
        int pos = i * 2 + DATA_OFFSET;
        frame.dataPoint[i] = (int)((frame.data[pos] & 0x0F) * 0x100 + frame.data[pos + 1]);
    }

    // if (frame.data[DATA_SIZE - 4] * 0x1000000 + frame.data[DATA_SIZE - 3] *
    // 0x10000 +
    //	frame.data[DATA_SIZE - 2] * 0x100 + frame.data[DATA_SIZE - 1] !=
    //	0xFFFFFFFF) {
    //	warning << "wrong ending of frame! Event Id : " << frame.evId << "
    // Channel Id : " << frame.channelId
    //		<< endl;
    //	return false;
    //}

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    return true;
}

bool TRestRawUSTCToSignalProcess::AddBuffer(USTCDataFrame& frame) {
#ifdef Incoherent_Event_Generation
    if (frame.evId == fCurrentEvent) {
        fEventBuffer[fCurrentBuffer].push_back(frame);
    } else {
        int pos = 1 + fCurrentBuffer;
        if (pos >= fEventBuffer.size()) pos -= fEventBuffer.size();
        fEventBuffer[pos].push_back(frame);
    }
#else
    if (frame.evId >= fCurrentEvent + (int)fEventBuffer.size()) {
        RESTWarning << "too large event id for buffering!" << RESTendl;
        RESTWarning << "this may due to the inconherence of event id. Increase the "
                       "buffer number!"
                    << RESTendl;
        RESTWarning << "Current Event, Burrfering event : " << fCurrentEvent << ", " << frame.evId
                    << RESTendl;
        return false;
    }
    if (frame.evId < fCurrentEvent) {
        RESTWarning << "skipping a signal from old event!" << RESTendl;
        RESTWarning << "the cause may be that too much events are mixing. Increase the "
                       "buffer number!"
                    << RESTendl;
        RESTWarning << "Current Event, Burrfering event : " << fCurrentEvent << ", " << frame.evId
                    << RESTendl;
        return false;
    }
    size_t pos = frame.evId - fCurrentEvent + fCurrentBuffer;
    if (pos >= fEventBuffer.size()) pos -= fEventBuffer.size();
    fEventBuffer[pos].push_back(frame);
#endif

    return true;
}

void TRestRawUSTCToSignalProcess::ClearBuffer() {
    fEventBuffer[fCurrentBuffer].clear();
    fCurrentBuffer += 1;
    if (fCurrentBuffer >= (int)fEventBuffer.size()) {
        fCurrentBuffer -= fEventBuffer.size();
    }
}

Bool_t TRestRawUSTCToSignalProcess::EndReading() {
    for (const auto& file : fInputFiles) {
        if (file != nullptr) {
            return false;
        }
    }

    for (const auto& eventBuffer : fEventBuffer) {
        if (!eventBuffer.empty()) {
            return false;
        }
    }

    return kTRUE;
}
