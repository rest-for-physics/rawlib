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

#ifndef RestCore_TRestBiPoToSignalProcess
#define RestCore_TRestBiPoToSignalProcess

#include "TRestRawSignalEvent.h"
#include "TRestRawToSignalProcess.h"

static const size_t CTAG_SZ = 4;

const int MATACQ_N_CH = 4;               // Number of channels
const int MATACQ_MAX_DATA_SAMP = 10240;  // Max number of samples for
const int MATACQ_BIPO_TIMEOUT = 0;       // BiPo mode timeout

static const int MATACQ_UNDERFLOW = 0x0000;
static const int MATACQ_ZERO = 0x8000;
static const int MATACQ_OVERFLOW = 0xFFFF;

const std::string TAG_RUN_START = "STA";
const std::string TAG_RUN_BIPO = "ST2";
const std::string TAG_RUN_STOP = "STO";
const std::string TAG_ACQ = "ACQ";
const std::string TAG_ACQ_2 = "AC2";

/// A structure to store the configuration settings of Matacq board
struct MatacqBoard {
    /// The base memory address of the Matacq board
    int32_t address;

    int32_t en_ch[MATACQ_N_CH];
    int32_t trg_ch[MATACQ_N_CH];

    int32_t Trig_Type;
    int32_t Threshold;
    int32_t Nb_Acq;
    int32_t Posttrig;
    int32_t Time_Tag_On;
    int32_t Sampling_GHz;

    int32_t ch_shifts[MATACQ_N_CH];
    int32_t nChannels;
};

/// A structure to store the BiPo settings
struct BiPoSettings {
    int32_t trigger_address;

    int32_t Win1_Posttrig;
    int32_t Timeout_200KHz;

    int32_t Trig_Chan[MATACQ_N_CH];
    int32_t Level1_mV[MATACQ_N_CH];
    int32_t Level2_mV[MATACQ_N_CH];

    int32_t t1_window;
    int32_t t2_window;
    int32_t t1_t2_timeout;
};

//! An process to read binary data from BiPo electronics
class TRestRawBiPoToSignalProcess : public TRestRawToSignalProcess {
   protected:
    /// The number of Matacq boards present on the setup
    Int_t fNBoards = 0;  ///<

    /// A vector of Matacq boards that contain the information of each card
    std::vector<MatacqBoard> fMatacqBoard;  //<

    /// A vector of BiPo settings
    std::vector<BiPoSettings> fBiPoSettings;  //<

    /// A temporary counter used to define the event id
    Int_t fEventCounter = 0;  //!

    void ReadHeader();
    void ReadFooter();
    void ReadBoard();
    void ReadBiPoSetup();
    Int_t ReadBiPoEventData(uint16_t* mdata);

    UInt_t GetBoardIndex(Int_t address);
    Int_t GetBin(Int_t boardIndex, Int_t channel, Int_t bin);

   public:
    void InitProcess() override;
    void Initialize() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    const char* GetProcessName() const override { return "BiPoToSignal"; }

    void PrintMetadata() override;

    MatacqBoard GetMatacqBoard(Int_t n) { return n > fNBoards ? (MatacqBoard){} : fMatacqBoard[n]; }
    BiPoSettings GetBiPoSettings(Int_t n) { return n > fNBoards ? (BiPoSettings){} : fBiPoSettings[n]; }

    // Constructor
    TRestRawBiPoToSignalProcess();

    // Destructor
    ~TRestRawBiPoToSignalProcess();

    ClassDefOverride(TRestRawBiPoToSignalProcess, 1);
};

#endif
