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

#ifndef RestCore_TRestFEUDreamToSignalProcess
#define RestCore_TRestFEUDreamToSignalProcess

#include <TError.h>
#include <TF1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TTree.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "TRestRawSignalEvent.h"
#include "TRestRawToSignalProcess.h"
#include "math.h"

//! An process to read binary data from FEUDream electronics
class TRestRawFEUDreamToSignalProcess : public TRestRawToSignalProcess {
   protected:
    class DataLineDream {
       public:
        DataLineDream() { data = 0; }
        ~DataLineDream() {}
        void ntohs_() { data = ntohs(data); };
        bool is_final_trailer() const { return (((data)&0x7000) >> 12) == 7; }   // X111
        bool is_end_of_event() const { return (((data)&0x7800) >> 11) == 0xF; }  // X1111
        bool is_data_trailer() const { return (((data)&0x6000) >> 13) == 2; }    // X10X
        bool is_first_line() const { return (((data)&0x7000) >> 12) == 3; }      // X011
        bool is_data() const { return (((data)&0x7000) >> 12) == 0; }            // X000
        bool is_data_zs() const { return (((data)&0x6000) >> 13) == 0; }         // X00X
        bool is_channel_ID() const { return (((data)&0x7000) >> 12) == 1; }      // X001
        bool is_Feu_header() const { return (((data)&0x7000) >> 12) == 6; }      // X110
        bool is_data_header() const { return (((data)&0x6000) >> 13) == 1; }     // X01X
        bool get_zs_mode() const { return (((data)&0x400) >> 10); }
        int get_Feu_ID() const { return (((data)&0xFF)); }
        long int get_finetstp() const { return (((data)&0x0007)); }
        //#define GET_FINETSTP(word)      (word & 0x0007)
        int get_sample_ID() const { return (((data)&0xFF8) >> 3); }
        int get_channel_ID() const { return (((data)&0x3F)); }
        int get_dream_ID() const { return (((data)&0xE00) >> 9); }     // non-zS mode
        int get_dream_ID_ZS() const { return (((data)&0xE00) >> 6); }  // zS mode
        int get_TimeStamp_Op() const { return (((data)&0x1FF)); }
        int get_data() const { return (((data)&0xFFF)); }
        unsigned short int data;
    };

    class FeuReadOut {
       public:
        int Id = -1;
        int N = -1;
        DataLineDream current_data;
        bool data_to_treat = false;
        bool event_completed = false;
        bool last_event = false;
        bool FeuHeaderLoaded;
        int FeuHeaderLine = 0;
        int DataHeaderLine = 0;
        int DataTrailerLine = 0;
        int asicN = -1;
        int channelN = 0;
        int channel_data;
        int physChannel = 0;
        int EventID = -1;
        int EventID_Op = -1;
        int TriggerID = -1;
        int TriggerID_ISB = -1;
        int TriggerID_LSB = -1;
        int CMN = -1;
        int CMN_rest = -1;
        long int Cell_ID = -1;
        int Cell_ID_ISB = -1;
        int Cell_ID_LSB = -1;
        int Cell_ID_MSB = -1;
        long int TimeStamp = -1;
        long int TimeStamp_Op1 = -1;
        long int TimeStamp_Op2 = -1;
        long int TimeStamp_Op3 = -1;
        long int FineTimeStamp = -1;
        int isample = -1;
        int isample_prev = -2;
        bool zs_mode = false;

        FeuReadOut() {}

        void NewEvent() {
            FeuHeaderLoaded = false;
            EventID = -1;
            event_completed = false;
            isample = -1;
            isample_prev = -2;
            FeuHeaderLine = 0;
            DataHeaderLine = 0;
            FineTimeStamp = -1;
            TimeStamp = -1;
            Id = -1;
        }
    };

    // unsigned int fLastEventId;
    // Double_t fLastTimeStamp;
    const int NstripMax = 64;  // number of strips on each chip
    const int MaxPhysChannel = 512;
    bool bad_event;         // flag to tag bad event
    int line;               // line number
    int Nevent, Nbadevent;  // current event number
    int IDEvent = 0;        // ID of event in Feu header
                            // double MaxThreshold;

   public:
    bool ReadFeuHeaders(FeuReadOut& feu);
    bool ReadDreamData(FeuReadOut& feu);
    bool ReadFeuTrailer(FeuReadOut& feu);
    bool ReadEvent(FeuReadOut& feu);

    void InitProcess();
    void Initialize();
    TRestEvent* ProcessEvent(TRestEvent* evInput);
    inline TString GetProcessName() const { return (TString) "DreamToSignal"; }

    // Constructor
    TRestRawFEUDreamToSignalProcess();
    TRestRawFEUDreamToSignalProcess(char* configFilename);

    // Destructor
    ~TRestRawFEUDreamToSignalProcess();

    ClassDef(TRestRawFEUDreamToSignalProcess, 1);
};

#endif
