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

#ifndef RestCore_TRestRawAFTERToSignalProcess
#define RestCore_TRestRawAFTERToSignalProcess

#include "TRestRawToSignalProcess.h"

// ATENTION: new T2K Daq versions 2.X need to read one extra word
#define NEW_DAQ_T2K_2_X

//--------------------------------------------------------
// Structure acquisition data
//--------------------------------------------------------
struct EventHeader {
    uint32_t eventSize;
    uint32_t eventNumb;
    // int eventTime;
    // uint16_t dummy;
};

// ATENTION!!!!!
// New verison of the DaqT2K (2.x)
// added 30th July 2012 (JuanAn)
struct DataPacketHeader {
    uint16_t size;
#ifdef NEW_DAQ_T2K_2_X
    uint16_t dcc;
#endif
    uint16_t hdr;
    uint16_t args;
    uint16_t ts_h;
    uint16_t ts_l;
    uint16_t ecnt;
    uint16_t scnt;
};

struct DataPacketEnd {
    uint16_t crc1;
    uint16_t crc2;
};

//! A process to read binary files produced with AFTER electronics
class TRestRawAFTERToSignalProcess : public TRestRawToSignalProcess {
   protected:
    unsigned int prevTime;
    double reducedTime;

   public:
    void Initialize();
    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* evInput);
    TString GetProcessName() { return (TString) "AFTERToSignal"; }
    TRestMetadata* GetProcessMetadata() { return NULL; }

    Bool_t isExternal() { return true; }

    TRestRawAFTERToSignalProcess();
    ~TRestRawAFTERToSignalProcess();

    ClassDef(TRestRawAFTERToSignalProcess, 1);
};
#endif
