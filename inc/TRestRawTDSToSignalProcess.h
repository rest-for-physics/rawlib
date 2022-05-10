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

#ifndef RestCore_TRestRawTDSToSignalProcess
#define RestCore_TRestRawTDSToSignalProcess

#include "TRestRawToSignalProcess.h"

//Struct that holds block header TDS data format
struct ANABlockHead {
    uint64_t TimeStamp;
    uint64_t RTTicks;
    uint64_t LTTicks;
    uint64_t SRate;
    uint16_t PSize;
    uint16_t Pretrigger;
    uint16_t mVdiv[4];
    uint16_t NEvents;
    uint16_t NHits;
    uint8_t NegPolarity[4];
};

//Struct that holds event header TDS data format
struct ANAEventHead {
    uint64_t clockTicksLT;
    uint64_t clockTicksRT;
};

//! A process to read binary files produced with TDS (Tektronix oscilloscope) DAQ
class TRestRawTDSToSignalProcess : public TRestRawToSignalProcess {
   protected:
    //Time stamp for block header
    double tNow=0;        //!

    //Number of samples per block
    int nSamples;         //!
    
    //Number of acquired channels 
    int nChannels;        //!
    
    //Number of points per horizontal axis
    int pulseDepth;       //!
    
    //Event counter
    int nEvents = 0;      //!

    //Check if pulses are negative or positive
    bool negPolarity[4];  //!

    //Sampling rate in MHz
    double fRate = 0;

    //Vertical scale in mV/Division 
    double fScale[4] = {0, 0, 0, 0};

   public:
    void Initialize();
    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* evInput);
    TString GetProcessName() { return (TString) "TDSToSignal"; }
    TRestMetadata* GetProcessMetadata() { return nullptr; }

    Bool_t isExternal() { return true; }

    TRestRawTDSToSignalProcess();
    ~TRestRawTDSToSignalProcess();

    ClassDef(TRestRawTDSToSignalProcess, 1);
};
#endif
