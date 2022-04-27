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

#ifndef RestCore_TRestRawMultiCoBoAsAdToSignalProcess
#define RestCore_TRestRawMultiCoBoAsAdToSignalProcess

#include <map>

#include "TRestRawSignalEvent.h"
#include "TRestRawToSignalProcess.h"

struct CoBoDataFrame {
    CoBoDataFrame() {
        timeStamp = 0;
        evId = -1;
        asadId = -1;
        for (bool& m : chHit) m = kFALSE;
        for (auto& m : data) {
            for (int& l : m) {
                l = 0;
            }
        }
    }
    TTimeStamp timeStamp;
    Bool_t chHit[272];
    Int_t data[272][512];
    Int_t evId;  // if equals -1, this data frame is used but have not been
                 // re-filled
    Int_t asadId;
    Bool_t finished;
};

struct CoBoHeaderFrame {
    CoBoHeaderFrame() {
        for (unsigned char& m : frameHeader) {
            m = 0;
        }
        // 4294967295 == -1  --> ends reading
        // 4294967294 == -2  --> just initialized
        eventIdx = (unsigned int)4294967294;
    }
    UChar_t frameHeader[256];  // 256: size of header of the cobo data frame

    unsigned int frameSize;
    unsigned int frameType;
    unsigned int revision;
    unsigned int headerSize;
    unsigned int itemSize;
    unsigned int nItems;
    unsigned int eventIdx;
    unsigned int asadIdx;
    unsigned int readOffset;
    unsigned int status;

    Long64_t eventTime;
    // TTimeStamp fEveTimeStamp;
    // int fEveTimeSec;
    // int fEveTimeNanoSec;

    void Show() {
        std::cout << "------ Frame Header ------" << endl;
        std::cout << "frameSize " << frameSize << endl;

        std::cout << "frameType " << frameType << endl;
        std::cout << "revision " << revision << endl;
        std::cout << "headerSize " << headerSize << endl;
        std::cout << "itemSize " << itemSize << endl;
        std::cout << "nItems " << nItems << endl;
        std::cout << "eventTime " << eventTime << endl;
        std::cout << "eventIdx " << eventIdx << endl;

        std::cout << "asadIdx " << asadIdx << endl;
        std::cout << "readOffset " << readOffset << endl;
        std::cout << "status " << status << endl;
    }
};

class TRestRawMultiCoBoAsAdToSignalProcess : public TRestRawToSignalProcess {
   private:
#ifndef __CINT__
    TRestRawSignal signal;  //!

    UChar_t frameDataP[2048];    //!///for partial readout data frame
    UChar_t frameDataF[278528];  //!///for full readout data frame

    TTimeStamp fStartTimeStamp;  //!

    std::map<int, CoBoDataFrame> fDataFrame;  //!///asadId, dataframe

    std::vector<CoBoHeaderFrame> fHeaderFrame;  //!///reserves a header frame for each file

    int fCurrentEvent = -1;  //!
    int fNextEvent = -1;     //!
#endif

   public:
    void InitProcess();

    Bool_t AddInputFile(std::string file);

    void Initialize();

    Bool_t InitializeStartTimeStampFromFilename(TString fName);

    TRestEvent* ProcessEvent(TRestEvent* inputEvent);

    void EndProcess();

    Bool_t FillBuffer(Int_t n);

    bool fillbuffer();

    bool ReadFrameHeader(CoBoHeaderFrame& Frame);

    bool ReadFrameDataP(FILE* f, CoBoHeaderFrame& hdr);
    bool ReadFrameDataF(CoBoHeaderFrame& hdr);

    Bool_t EndReading();

    // Constructor
    TRestRawMultiCoBoAsAdToSignalProcess();
    TRestRawMultiCoBoAsAdToSignalProcess(const char* configFilename);
    // Destructor
    ~TRestRawMultiCoBoAsAdToSignalProcess();

    ClassDef(TRestRawMultiCoBoAsAdToSignalProcess, 1);
};
#endif
