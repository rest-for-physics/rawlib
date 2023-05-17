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

///////////////////////////////////////////////////////////////////////
// The TRestRawBiPoToSignalProcess ...
//
// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
//
// TODO: This process requires optimization to improve the data processing
// rate.
//
// <hr>
//
// \warning **⚠ REST is under continous development.** This documentation
// is offered to you by the REST community. Your HELP is needed to keep this
// code up to date. Your feedback will be worth to support this software, please
// report any problems/suggestions you may find will using it at [The REST Framework
// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
// updating information or adding/proposing new contributions. See also our
// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
// Guide</a>.
//
//
//--------------------------------------------------------------------------
//
// RESTsoft - Software for Rare Event Searches with TPCs
//
// History of developments:
//
// 2023-May: First implementation (from https://gitlab.in2p3.fr/bipo/matacqana.git)
//           Javier Galan
//
// \class      TRestRawBiPoToSignalProcess
// \author     Javier Galan
//
// <hr>
/////
#include "TRestRawBiPoToSignalProcess.h"

using namespace std;

#include "TTimeStamp.h"
#include "zlib.h"

ClassImp(TRestRawBiPoToSignalProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawBiPoToSignalProcess::TRestRawBiPoToSignalProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawBiPoToSignalProcess::~TRestRawBiPoToSignalProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the section name
///
void TRestRawBiPoToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    SetLibraryVersion(LIBRARY_VERSION);
}

///////////////////////////////////////////////
/// \brief Process initialization. Data members that require initialization just before start processing
/// should be initialized here.
///
void TRestRawBiPoToSignalProcess::InitProcess() {
    TRestRawToSignalProcess::InitProcess();

    fEventCounter = 0;

    tStart = 0;  // timeStamp of the run initially set to 0
    RESTInfo << "TRestRawBiPoToSignalProcess::InitProcess" << RESTendl;

    //// Validating the file type
    char buffer[CTAG_SZ];
    if (fread(buffer, sizeof(char), CTAG_SZ, fInputBinFile) != CTAG_SZ) {
        printf("Error: could not read first prefix.\n");
        exit(1);
    }
    totalBytesReaded += CTAG_SZ * sizeof(char);

    if (buffer != TAG_RUN_BIPO) {
        RESTError << "The file " << fInputFileNames[0] << " is not BiPo format" << RESTendl;
        exit(1);
    }

    /// Reading MATACQ boards and BiPo setup settings
    ReadHeader();
    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug) GetChar();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawBiPoToSignalProcess::ProcessEvent(TRestEvent* inputEvent) {
    RESTDebug << "-------Start of TRestRawBiPoToSignalProcess::ProcessEvent------------" << RESTendl;
    fEventCounter++;
    RESTDebug << "--- Starting to process event id: " << fEventCounter << RESTendl;

    /// Initializing the new signal event
    fSignalEvent->Initialize();
    fSignalEvent->SetRunOrigin(fRunOrigin);
    fSignalEvent->SetSubRunOrigin(fSubRunOrigin);
    fSignalEvent->SetID(fEventCounter);

    char buffer[CTAG_SZ];
    if (fread(buffer, sizeof(char), CTAG_SZ, fInputBinFile) != CTAG_SZ) {
        printf("Error: could not read first ACQ prefix.\n");
        exit(1);
    }
    totalBytesReaded += CTAG_SZ * sizeof(char);

    if (std::string(buffer) == TAG_RUN_STOP) {
        RESTDebug << "The run ends" << RESTendl;
        ReadFooter();
        // The processing thread finishes
        return nullptr;
    }

    if (std::string(buffer) == TAG_ACQ || std::string(buffer) == TAG_ACQ_2) {
        RESTDebug << "A new event comes" << RESTendl;

        uint16_t data[MATACQ_MAX_DATA_SAMP];
        Int_t boardAddress = ReadBiPoEventData(data);
        Int_t bIndex = GetBoardIndex(boardAddress);

        if (bIndex < 0) {
            RESTError << "TRestRawBiPoToSignalProcess::ProcessEvent." << RESTendl;
            RESTError << "Board index not found!" << RESTendl;
            return nullptr;
        }

        RESTDebug << "Number of channels : " << fMatacqBoard[bIndex].nChannels << RESTendl;
        for (int nch = 0; nch < fMatacqBoard[bIndex].nChannels; nch++) {
            TRestRawSignal sgnl;
            sgnl.Initialize();
            sgnl.SetSignalID(100 * boardAddress + nch);

            Int_t nBins = fBiPoSettings[bIndex].t1_window + fBiPoSettings[bIndex].t2_window;

            for (int b = 0; b < nBins; b++) {
                Short_t sdata = data[GetBin(bIndex, nch, b)];
                Short_t v = MATACQ_ZERO - sdata;  // Inversing polarity
                if (sdata == MATACQ_OVERFLOW) {
                    v = 0;
                }
                if (sdata == MATACQ_UNDERFLOW) {
                    v = TMath::Power(2, 12);
                }

                if (sgnl.GetSignalID() >= 0) sgnl.AddPoint(v);
            }

            RESTDebug << "Adding signal with id : " << sgnl.GetID() << RESTendl;
            RESTDebug << "Number of points: " << sgnl.GetNumberOfPoints() << RESTendl;
            fSignalEvent->AddSignal(sgnl);
        }

        return fSignalEvent;
    }

    // The processing thread will be finished if return nullptr is reached
    return nullptr;
}

///////////////////////////////////////////////
/// \brief This method reads the header data containing the run timestamp,
/// the number of Matacq boards, and the settings of each of the Matacq and
/// BiPo boards.
///
void TRestRawBiPoToSignalProcess::ReadFooter() {
    RESTDebug << "Entering TRestRawBiPoToSignalProcess::ReadFooter" << RESTendl;
    int32_t tmp;

    /// Reading the run start timestamp
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    Double_t runEndTime = (Double_t)tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp (us).\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    runEndTime += 1.e-6 * (Double_t)tmp;

    fRunInfo->SetEndTimeStamp(runEndTime);
}

///////////////////////////////////////////////
/// \brief This method reads the header data containing the run timestamp,
/// the number of Matacq boards, and the settings of each of the Matacq and
/// BiPo boards.
///
void TRestRawBiPoToSignalProcess::ReadHeader() {
    RESTDebug << "Entering TRestRawBiPoToSignalProcess::ReadHeader" << RESTendl;
    int32_t tmp;

    /// Reading the run start timestamp
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    Double_t runStartTime = (Double_t)tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp (us).\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    runStartTime += 1.e-6 * (Double_t)tmp;

    fRunInfo->SetStartTimeStamp(runStartTime);

    uint32_t nBoards;
    if (fread(&nBoards, sizeof(uint32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read nBoards.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);

    fNBoards = nBoards;
    RESTDebug << "N boards: " << fNBoards << RESTendl;

    for (int n = 0; n < fNBoards; n++) {
        ReadBoard();

        int32_t bipo;
        if (fread(&bipo, sizeof(int32_t), 1, fInputBinFile) != 1) {
            printf("Error: could not read BiPo flag.\n");
            exit(1);
        }
        totalBytesReaded += sizeof(int32_t);

        if (bipo != 1) {
            RESTError << "The file " << fInputFileNames[0] << " is not BiPo format" << RESTendl;
            exit(1);
        }

        ReadBiPoSetup();
    }
}

void TRestRawBiPoToSignalProcess::ReadBoard() {
    MatacqBoard board;
    int32_t tmp;
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.address = tmp;

    if (fread(board.en_ch, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read base matacq en_ch.\n");
        exit(1);
    }
    totalBytesReaded += MATACQ_N_CH * sizeof(int32_t);

    int cnt = 0;
    board.nChannels = 0;
    for (int ich = (MATACQ_N_CH - 1); ich >= 0; ich--) {
        if (board.en_ch[ich] == 1) {
            board.nChannels = board.nChannels + 1;
            board.ch_shifts[ich] = cnt;
            cnt++;
        } else {
            board.ch_shifts[ich] = -1;
        }
    }

    if (fread(board.trg_ch, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read base matacq trg_ch.\n");
        exit(1);
    }
    totalBytesReaded += MATACQ_N_CH * sizeof(int32_t);

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Trig type.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Trig_Type = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Threshold.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Threshold = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Nb_Acq.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Nb_Acq = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Posttrig.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Posttrig = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Time_Tag_On.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Time_Tag_On = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read Sampling_GHz.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    board.Sampling_GHz = tmp;

    RESTDebug << "MATACQ Base memory address: " << board.address << RESTendl;
    RESTDebug << "En[0]: " << board.en_ch[0] << " En[1]: " << board.en_ch[1] << " En[2]: " << board.en_ch[2]
              << " En[3]: " << board.en_ch[3] << RESTendl;
    RESTDebug << "Trg[0]: " << board.trg_ch[0] << " Trg[1]: " << board.trg_ch[1]
              << " Trg[2]: " << board.trg_ch[2] << " Trg[3]: " << board.trg_ch[3] << RESTendl;
    RESTDebug << " " << RESTendl;
    RESTDebug << "Trigger type: " << board.Trig_Type << " Threshold: " << board.Threshold << RESTendl;
    RESTDebug << "Nb_Acq: " << board.Nb_Acq << " Posttrig: " << board.Posttrig << RESTendl;
    RESTDebug << "Time_Tag_On: " << board.Time_Tag_On << " Sampling_GHz: " << board.Sampling_GHz << RESTendl;
    RESTDebug << " --  " << RESTendl;

    fMatacqBoard.push_back(board);
}

///////////////////////////////////////////////
/// \brief This method reads the header data corresponding to the
/// BiPo settings.
///
void TRestRawBiPoToSignalProcess::ReadBiPoSetup() {
    BiPoSettings bipo;

    int32_t tmp;
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo trigger address.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.trigger_address = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.Win1_Posttrig = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo timeout 200KHz.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.Timeout_200KHz = tmp;

    if (fread(bipo.Trig_Chan, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Trig_Chan.\n");
        exit(1);
    }
    totalBytesReaded += MATACQ_N_CH * sizeof(int32_t);

    if (fread(bipo.Level1_mV, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Level1_mV.\n");
        exit(1);
    }
    totalBytesReaded += MATACQ_N_CH * sizeof(int32_t);

    if (fread(bipo.Level2_mV, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Level2_mV.\n");
        exit(1);
    }
    totalBytesReaded += MATACQ_N_CH * sizeof(int32_t);

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.t1_window = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.t2_window = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    bipo.t1_t2_timeout = tmp;

    RESTDebug << "BiPo trigger address: " << bipo.trigger_address << RESTendl;
    RESTDebug << "Win1 Posttrig: " << bipo.Win1_Posttrig << RESTendl;
    RESTDebug << "Timeout [200KHz]: " << bipo.Timeout_200KHz << RESTendl;
    RESTDebug << " " << RESTendl;
    RESTDebug << "Trig_Chan[0]: " << bipo.Trig_Chan[0] << " Trig_Chan[1]: " << bipo.Trig_Chan[1]
              << " Trig_Chan[2]: " << bipo.Trig_Chan[2] << " Trig_Chan[3]: " << bipo.Trig_Chan[3] << RESTendl;
    RESTDebug << "Level1_mV[0]: " << bipo.Level1_mV[0] << " Level1_mV[1]: " << bipo.Level1_mV[1]
              << " Level1_mV[2]: " << bipo.Level1_mV[2] << " Level1_mV[3]: " << bipo.Level1_mV[3] << RESTendl;
    RESTDebug << "Level2_mV[0]: " << bipo.Level2_mV[0] << " Level2_mV[1]: " << bipo.Level2_mV[1]
              << " Level2_mV[2]: " << bipo.Level2_mV[2] << " Level2_mV[3]: " << bipo.Level2_mV[3] << RESTendl;
    RESTDebug << " " << RESTendl;
    RESTDebug << "T1 window: " << bipo.t1_window << RESTendl;
    RESTDebug << "T2 window: " << bipo.t2_window << RESTendl;
    RESTDebug << "T1-T2 timeout: " << bipo.t1_t2_timeout << RESTendl;
    RESTDebug << " --  " << RESTendl;

    fBiPoSettings.push_back(bipo);
}

///////////////////////////////////////////////
/// \brief This method reads the event data corresponding to one event.
/// The sampled channel data that will be made accessible at the `mdata`
/// pointer provided as argument.
///
/// The event timestamp and the triggered board values will be also read
/// here. The event timestamp will be assigned to the fSignalEvent, while
/// the triggered board address will be returned and it will be used
/// later on to generate a signal id.
///
Int_t TRestRawBiPoToSignalProcess::ReadBiPoEventData(uint16_t* mdata) {
    int32_t tmp;
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    Int_t boardAddress = tmp;

    RESTDebug << " Event address --> " << boardAddress << RESTendl;

    //   int32_t event_address = tmp; // It is this important?
    //   Probably board where it took place the event?

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);

    Double_t timeStamp = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    timeStamp += 1.e-6 * (Double_t)tmp;

    fSignalEvent->SetTime(timeStamp);

    RESTDebug << "Event time stamp: " << timeStamp << RESTendl;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    int32_t data_size = tmp;
    RESTDebug << "Data size --> " << tmp << RESTendl;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo trigger address.\n");
        exit(1);
    }
    totalBytesReaded += sizeof(int32_t);
    RESTDebug << " T1-T2 distance --> " << tmp << RESTendl;

    if (fread(mdata, sizeof(uint16_t), data_size, fInputBinFile) != (size_t)data_size) {
        printf("Error: could not read MATACQ data.\n");
        exit(1);
    }
    totalBytesReaded += data_size * sizeof(uint16_t);

    return boardAddress;
}

UInt_t TRestRawBiPoToSignalProcess::GetBoardIndex(Int_t address) {
    for (unsigned int n = 0; n < fMatacqBoard.size(); n++)
        if (fMatacqBoard[n].address == address) return n;

    return -1;
}

Int_t TRestRawBiPoToSignalProcess::GetBin(Int_t boardIndex, Int_t channel, Int_t bin) {
    MatacqBoard board = fMatacqBoard[boardIndex];
    return board.ch_shifts[channel] + board.nChannels * bin;
}

///////////////////////////////////////////////
/// \brief Prints out the Matacq boards configuration and BiPo setup
///
void TRestRawBiPoToSignalProcess::PrintMetadata() {
    TRestMetadata::PrintMetadata();

    RESTMetadata << "Number of Matacq boards : " << fNBoards << RESTendl;
    RESTMetadata << " " << RESTendl;

    for (int n = 0; n < fNBoards; n++) {
        RESTMetadata << " " << RESTendl;
        RESTMetadata << "Board address: " << fMatacqBoard[n].address << RESTendl;
        RESTMetadata << "----" << RESTendl;
        RESTMetadata << " - Enabled channels: " << fMatacqBoard[n].en_ch[0] << " - "
                     << fMatacqBoard[n].en_ch[1] << " - " << fMatacqBoard[n].en_ch[2] << " - "
                     << fMatacqBoard[n].en_ch[3] << RESTendl;
        RESTMetadata << " - Trigger channels: " << fMatacqBoard[n].trg_ch[0] << " - "
                     << fMatacqBoard[n].trg_ch[1] << " - " << fMatacqBoard[n].trg_ch[2] << " - "
                     << fMatacqBoard[n].trg_ch[3] << RESTendl;
        RESTMetadata << " - Trigger type: " << fMatacqBoard[n].Trig_Type << RESTendl;
        RESTMetadata << " - Threshold: " << fMatacqBoard[n].Threshold << RESTendl;
        RESTMetadata << " - Nb_Acq: " << fMatacqBoard[n].Nb_Acq << RESTendl;
        RESTMetadata << " - Posttrig: " << fMatacqBoard[n].Posttrig << RESTendl;
        RESTMetadata << " - Time_Tag_On: " << fMatacqBoard[n].Time_Tag_On << RESTendl;
        RESTMetadata << " - Sampling_GHz: " << fMatacqBoard[n].Sampling_GHz << RESTendl;
        RESTMetadata << " " << RESTendl;
        RESTMetadata << "BiPo trigger settings. Address : " << fBiPoSettings[n].trigger_address << RESTendl;
        RESTMetadata << "----" << RESTendl;
        RESTMetadata << " - Win1 Posttrig: " << fBiPoSettings[n].Win1_Posttrig << RESTendl;
        RESTMetadata << " - Timeout [200KHz]: " << fBiPoSettings[n].Timeout_200KHz << RESTendl;
        RESTMetadata << " - Trigger channels: " << fBiPoSettings[n].Trig_Chan[0] << " - "
                     << fBiPoSettings[n].Trig_Chan[1] << " - " << fBiPoSettings[n].Trig_Chan[2] << " - "
                     << fBiPoSettings[n].Trig_Chan[3] << RESTendl;
        RESTMetadata << " - Level 1 [mV]: " << fBiPoSettings[n].Level1_mV[0] << " - "
                     << fBiPoSettings[n].Level1_mV[1] << " - " << fBiPoSettings[n].Level1_mV[2] << " - "
                     << fBiPoSettings[n].Level1_mV[3] << RESTendl;
        RESTMetadata << " - Level 2 [mV]: " << fBiPoSettings[n].Level2_mV[0] << " - "
                     << fBiPoSettings[n].Level2_mV[2] << " - " << fBiPoSettings[n].Level2_mV[2] << " - "
                     << fBiPoSettings[n].Level2_mV[3] << RESTendl;
        RESTMetadata << " - T1 window: " << fBiPoSettings[n].t1_window << RESTendl;
        RESTMetadata << " - T2 window: " << fBiPoSettings[n].t2_window << RESTendl;
        RESTMetadata << " - T1-T2 timeout: " << fBiPoSettings[n].t1_t2_timeout << RESTendl;
    }

    RESTMetadata << "+++++++++++++++++++++++++++++++++++++++++++++++++" << RESTendl;
}
