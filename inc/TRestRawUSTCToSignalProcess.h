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

#ifndef RestCore_TRestRawUSTCToSignalProcess
#define RestCore_TRestRawUSTCToSignalProcess

#include <map>

#include "TRestRawToSignalProcess.h"

//#define V3_Readout_Format_Long
#define V4_Readout_Format
//#define Incoherent_Event_Generation

#ifdef V3_Readout_Format_Long
#define DATA_SIZE 1048
#define DATA_OFFSET (DATA_SIZE - 512 * 2 - 4)
#define PROTOCOL_SIZE 4
#endif

#ifdef V3_Readout_Format_Short
#define DATA_SIZE 1040
#define DATA_OFFSET (DATA_SIZE - 512 * 2 - 4)
#define PROTOCOL_SIZE 4
#endif

#ifdef V4_Readout_Format
#define DATA_SIZE 1036
#define DATA_OFFSET 6
#define HEADER_SIZE 36
#define ENDING_SIZE 16
#define PROTOCOL_SIZE 4
#endif

struct USTCDataFrame {
    // a signal-level data frame
    // e.g.
    // EEEE | E0A0 | 246C0686 | 0001 | 2233 | (A098)(A09C)... | FFFF
    // header | board number | event time | channel id(0~63) | event id | [chip id
    // + data(0~4095)]*512 | ending
    USTCDataFrame() {
        boardId = 0;
        chipId = 0;
        readoutType = 0;
        eventTime = 0;
        channelId = 0;
        evId = -1;
        signalId = 0;
    }
    UChar_t data[1048];  // the size of a signal frame

    Int_t boardId;       // 0~n
    Int_t chipId;        // 0~3 aget number
    Int_t readoutType;   // 2:full readout  1:partial readout
    Long64_t eventTime;  // event time in na
    Int_t channelId;     // 0~63 channels
    Int_t evId;          // if equals -1, this data frame is used but have not been
                         // re-filled

    Int_t signalId;
    Int_t dataPoint[512];
};

//! A process to read USTC electronic binary format files generated.
class TRestRawUSTCToSignalProcess : public TRestRawToSignalProcess {
   private:
#ifndef __CINT__
    TRestRawSignal sgnl;  //!

    UChar_t fHeader[64];
    UChar_t fEnding[32];

    std::vector<std::vector<USTCDataFrame>> fEventBuffer;  //!
    int nBufferedEvent;                                    //!
    int fCurrentFile = 0;                                  //!
    int fCurrentEvent = -1;                                //!
    int fCurrentBuffer = 0;                                //!
    int fLastBufferedId = 0;                               //!
    std::vector<int> errorevents;                          //!
    int unknownerrors = 0;                                 //!

    Long64_t fTimeOffset = 0;
    std::set<int> fChannelOffset;
#endif

   public:
    void InitProcess() override;
    void Initialize();

    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;

    void EndProcess() override;

    bool FillBuffer();

    bool GetNextFrame(USTCDataFrame&);

    bool OpenNextFile(USTCDataFrame&);

    void FixToNextFrame(FILE* f);

    bool ReadFrameData(USTCDataFrame& Frame);

    bool AddBuffer(USTCDataFrame& Frame);

    void ClearBuffer();

    Bool_t EndReading();

    // Constructor
    TRestRawUSTCToSignalProcess();
    TRestRawUSTCToSignalProcess(const char* configFilename);
    // Destructor
    ~TRestRawUSTCToSignalProcess();

    ClassDef(TRestRawUSTCToSignalProcess, 3);
};
#endif
