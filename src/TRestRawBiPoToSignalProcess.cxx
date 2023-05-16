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
// \warning **⚠ REST is under continous development.** This
// documentation
// is offered to you by the REST community. Your HELP is needed to keep this
// code
// up to date. Your feedback will be worth to support this software, please
// report
// any problems/suggestions you may find will using it at [The REST Framework
// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
// updating
// information or adding/proposing new contributions. See also our
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

TRestRawBiPoToSignalProcess::TRestRawBiPoToSignalProcess() { Initialize(); }

TRestRawBiPoToSignalProcess::TRestRawBiPoToSignalProcess(const char* configFilename)
    : TRestRawToSignalProcess(configFilename) {
    Initialize();
}

TRestRawBiPoToSignalProcess::~TRestRawBiPoToSignalProcess() {
    // TRestRawBiPoToSignalProcess destructor
}

void TRestRawBiPoToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    /*
    bad_event = false;
    line = 0;                   // line number
    Nevent = 0, Nbadevent = 0;  // current event number
    IDEvent = 0;
    */
}

void TRestRawBiPoToSignalProcess::InitProcess() {
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
}

TRestEvent* TRestRawBiPoToSignalProcess::ProcessEvent(TRestEvent* inputEvent) {
    RESTDebug << "-------Start of TRestRawBiPoToSignalProcess::ProcessEvent------------" << RESTendl;

    char buffer[CTAG_SZ];
    if (fread(buffer, sizeof(char), CTAG_SZ, fInputBinFile) != CTAG_SZ) {
        printf("Error: could not read first ACQ prefix.\n");
        exit(1);
    }

    if (std::string(buffer) == TAG_RUN_STOP) {
        RESTDebug << "The run ends" << RESTendl;
        return nullptr;
    }

    if (std::string(buffer) == TAG_ACQ || std::string(buffer) == TAG_ACQ_2) {
        RESTDebug << "A new event comes" << RESTendl;

        fSignalEvent->Initialize();
        uint16_t data[MATACQ_MAX_DATA_SAMP];

        ReadBiPoEventData(data);

        return fSignalEvent;
    }

    std::cout << "Buffer : " << buffer << std::endl;

    return nullptr;  // can't read data
}

void TRestRawBiPoToSignalProcess::ReadHeader() {
    RESTDebug << "Entering TRestRawBiPoToSignalProcess::ReadHeader" << RESTendl;
    int32_t tmp;

    /// Reading the run timestamp
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp.\n");
        exit(1);
    }
    fRunStartTime = (Double_t)tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read timestamp (us).\n");
        exit(1);
    }

    fRunStartTime += 1.e-6 * (Double_t)tmp;

    std::cout.precision(12);
    RESTDebug << "Run start time: " << fRunStartTime << RESTendl;

    uint32_t nBoards;
    if (fread(&nBoards, sizeof(uint32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read nBoards.\n");
        exit(1);
    }
    fNBoards = nBoards;
    RESTDebug << "N boards: " << fNBoards << RESTendl;

    for (int n = 0; n < fNBoards; n++) {
        ReadBoard();

        int32_t bipo;
        if (fread(&bipo, sizeof(int32_t), 1, fInputBinFile) != 1) {
            printf("Error: could not read BiPo flag.\n");
            exit(1);
        }

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
    board.address = tmp;

    if (fread(board.en_ch, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read base matacq en_ch.\n");
        exit(1);
    }

    if (fread(board.trg_ch, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read base matacq trg_ch.\n");
        exit(1);
    }

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    board.Trig_Type = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    board.Threshold = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    board.Nb_Acq = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    board.Posttrig = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
    board.Time_Tag_On = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read base matacq address.\n");
        exit(1);
    }
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
}

void TRestRawBiPoToSignalProcess::ReadBiPoSetup() {
    BiPoSettings bipo;

    int32_t tmp;
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo trigger address.\n");
        exit(1);
    }
    bipo.trigger_address = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    bipo.Win1_Posttrig = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo timeout 200KHz.\n");
        exit(1);
    }
    bipo.Timeout_200KHz = tmp;

    if (fread(bipo.Trig_Chan, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Trig_Chan.\n");
        exit(1);
    }

    if (fread(bipo.Level1_mV, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Level1_mV.\n");
        exit(1);
    }

    if (fread(bipo.Level2_mV, sizeof(int32_t), MATACQ_N_CH, fInputBinFile) != MATACQ_N_CH) {
        printf("Error: could not read Level2_mV.\n");
        exit(1);
    }

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    bipo.t1_window = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
    bipo.t2_window = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo Win1 Posttrig.\n");
        exit(1);
    }
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
}

Double_t TRestRawBiPoToSignalProcess::ReadBiPoEventData(uint16_t* mdata) {
    int32_t tmp;
    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    std::cout << " Event address --> " << tmp << std::endl;

    //   int32_t event_address = tmp; // It is this important?
    //   Probably board where it took place the event?

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }

    Double_t eventTimeStamp = tmp;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    eventTimeStamp += 1.e-6 * (Double_t)tmp;

    std::cout.precision(12);
    std::cout << "Event time stamp: " << eventTimeStamp << std::endl;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read  tmp .\n");
        exit(1);
    }
    int32_t data_size = tmp;
    std::cout << "Data size --> " << tmp << std::endl;

    if (fread(&tmp, sizeof(int32_t), 1, fInputBinFile) != 1) {
        printf("Error: could not read BiPo trigger address.\n");
        exit(1);
    }
    std::cout << " T1-T2 distance --> " << tmp << std::endl;

    if (fread(mdata, sizeof(uint16_t), data_size, fInputBinFile) != (size_t)data_size) {
        printf("Error: could not read MATACQ data.\n");
        exit(1);
    }

    for (int n = 0; n < data_size; n++) {
        std::cout << n << " : " << mdata[n] << std::endl;
    }

    return eventTimeStamp;
}
