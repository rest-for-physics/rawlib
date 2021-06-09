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
/// The TRestRawAFTERToSignalProcess is a process used to read a binary file
/// produced by the AFTER electronics, the resulting signal will be registered
/// inside a TRestRawSignalEvent, and then it processing can continue inside
/// with the REST framework libraries.
///
/// **TODO**: This process might be obsolete today. It may need additional revision,
/// validation, and documentation.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this code
/// up to date. Your feedback will be worth to support this software, please report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating information or adding/proposing new contributions. See also our
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
/// 2015-Oct: First implementation as part of the conceptualization of existing
///			  REST software.
///           Juanan Garcia
///
/// \class      TRestRawAFTERToSignalProcess
/// \author     Juanan Garcia
///
/// <hr>
///

#include "TRestRawAFTERToSignalProcess.h"
#include <bitset> 

#include "TTimeStamp.h"
#ifdef WIN32
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif
#include "mygblink.h"

ClassImp(TRestRawAFTERToSignalProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawAFTERToSignalProcess::TRestRawAFTERToSignalProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// The default behaviour is that the config file must be specified with
/// full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
///

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawAFTERToSignalProcess::~TRestRawAFTERToSignalProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
/// In this case we re-use the initialization of TRestRawToSignalProcess
/// interface class.
///
void TRestRawAFTERToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    prevTime = 0;
    reducedTime = 0;
}

///////////////////////////////////////////////
/// \brief Process initialization.
///
/// TODO It should adopt standard way to retrieve data from filename
/// and fill/update a TRestDetector metadata class. See for example
/// TRestRawMultiFEMINOSToSignalProcess.
///
void TRestRawAFTERToSignalProcess::InitProcess() {
    // Binary file header

    // The binary starts here
    char runUid[21], initTime[21];
    fread(runUid, 1, 20, fInputBinFile);
    runUid[20]='\0';
    sprintf(initTime, "%s", runUid);
    printf("File UID is %s \n", initTime);
    totalBytesReaded = sizeof(runUid);

    int year, day, month, hour, minute, second;
    sscanf(runUid, "R%d.%02d.%02d-%02d:%02d:%02d", &year, &month, &day, &hour, &minute, &second);
    printf("R%d_%02d_%02d-%02d_%02d_%02d\n", year, month, day, hour, minute, second);
    TTimeStamp tS(year, month, day, hour, minute, second);
    tStart = tS.AsDouble();
    cout << tStart << endl;
    // Timestamp of the run
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawAFTERToSignalProcess::ProcessEvent(TRestEvent* evInput) {
    EventHeader head;
    DataPacketHeader pHeader;
    DataPacketEnd pEnd;

    fSignalEvent->Initialize();

    // Read next header or quit of end of file
    if (fread(&head, sizeof(EventHeader), 1, fInputBinFile) != 1) {
        fclose(fInputBinFile);
        cout << "Error reading event header :-(" << endl;
        cout << "... or end of file found :-)" << endl;
        return nullptr;
    }

    head.eventSize = ntohl(head.eventSize);
    head.eventNumb = ntohl(head.eventNumb);
    
    payload = head.eventSize;
    frameBits = sizeof(head);

        debug << "Event number from header --> 0x"<< std::hex<< head.eventNumb<<std::dec << endl;
        debug << " event header size " << sizeof(head) << endl;
        debug << " total rawdata size 0x" << std::hex<< head.eventSize<< std::dec << endl;
        debug << "Payload " << payload << endl;

    fSignalEvent->SetID(head.eventNumb);

    int timeBin = 0;

    int fecN;
    int channel;
    int asicN;
    int physChannel;

    uint32_t eventTime, deltaTime;
    uint32_t th, tl;
    int tempAsic1, tempAsic2, sampleCountRead, pay;
    uint16_t data, dat;

    bool isData = false;

    bool first = true;

    // Bucle till it finds the readed bits equals the payload
    while (frameBits < payload) {
        fread(&pHeader, sizeof(DataPacketHeader), 1, fInputBinFile);
        frameBits += sizeof(DataPacketHeader);

        if (first)  // Timestamping (A. Tomas, 30th June 2011)
        {
            th = ntohs(pHeader.ts_h);
            tl = ntohs(pHeader.ts_l);
            eventTime = th <<16 | tl;  // built time from MSB and LSB

            if (eventTime > prevTime)
                deltaTime = eventTime - prevTime;
            else
                deltaTime = (0xFFFFFFFF - prevTime) + eventTime;

            reducedTime += deltaTime;

            // Set timestamp and event ID
            fSignalEvent->SetTime(tStart + reducedTime * 2.E-8);  // Set TimeStamp

            prevTime = eventTime;
            first = false;

            debug << "Timestamp: " << eventTime << endl;
        }

            debug << "******Event data packet header:******" << endl;

            debug << "Size " << ntohs(pHeader.size) << endl;

            debug << "Event data packet header: " << endl;
            debug << std::hex<<"Size 0x" << ntohs(pHeader.size) << endl;
#ifdef NEW_DAQ_T2K_2_X
            debug <<"DCC 0x" << ntohs(pHeader.dcc) << endl;
#endif
            debug <<"Hdr word 0x" << ntohs(pHeader.hdr) << endl;
            debug <<"Args 0x" << ntohs(pHeader.args) << endl;
            debug <<"TS_H 0x" << ntohs(pHeader.ts_h) << endl;
            debug <<"TS_L 0x" << ntohs(pHeader.ts_l) << endl;
            debug <<"Ecnt 0x" << ntohs(pHeader.ecnt) << endl;
            debug <<"Scnt 0x" << ntohs(pHeader.scnt) <<std::dec<< endl;

#ifdef NEW_DAQ_T2K_2_X
        debug << "RawDCC Head 0x" <<std::hex << ntohs(pHeader.dcc)<<std::dec << " Version " << GET_EVENT_TYPE(ntohs(pHeader.dcc));
        debug << " Flag " << ((ntohs(pHeader.dcc) & 0x3000) >> 12);
        debug << " RT " << ((ntohs(pHeader.dcc) & 0x0C00) >> 10) << " DCCInd "
                 << ((ntohs(pHeader.dcc) & 0x03F0) >> 4);
        debug << " FEMInd " << (ntohs(pHeader.dcc) & 0x000F) << endl;

        debug << "FEM0Ind " << ntohs(pHeader.hdr) << " Type " << ((ntohs(pHeader.hdr) & 0xF000) >> 12);
        debug << " L " << ((ntohs(pHeader.hdr) & 0x0800) >> 11);
        debug << " U " << ((ntohs(pHeader.hdr) & 0x0800) >> 10) << " FECFlags "
                 << ((ntohs(pHeader.hdr) & 0x03F0) >> 4);
        debug << " Index " << (ntohs(pHeader.hdr) & 0x000F) << endl;

        debug << "RawFEM 0x"<<std::hex << ntohs(pHeader.args)<<std::dec << " M " << ((ntohs(pHeader.args) & 0x8000) >> 15);
        debug << " N " << ((ntohs(pHeader.args) & 0x4000) >> 14) << " Zero "
                 << ((ntohs(pHeader.args) & 0x1000) >> 13);
        debug << " Arg2 " << GET_RB_ARG2(ntohs(pHeader.args)) << " Arg2 "
                 << GET_RB_ARG1(ntohs(pHeader.args)) << endl;
        debug << "TimeStampH " << ntohs(pHeader.ts_h) << endl;
        debug << "TimeStampL " << ntohs(pHeader.ts_l) << endl;
        debug << "RawEvType 0x"<<std::hex << ntohs(pHeader.ecnt)<<std::dec << " EvTy " << GET_EVENT_TYPE(ntohs(pHeader.ecnt));
        debug << " EventCount " << GET_EVENT_COUNT(ntohs(pHeader.ecnt)) << endl;
        debug << "Samples " << ntohs(pHeader.scnt) << endl;
#endif

        tempAsic1 = GET_RB_ARG1(ntohs(pHeader.args));
        tempAsic2 = GET_RB_ARG2(ntohs(pHeader.args));
        channel = tempAsic1 / 6;
        asicN = (10 * (tempAsic1 % 6) / 2 + tempAsic2) % 4;
        fecN = (10 * (tempAsic1 % 6) / 2 + tempAsic2) / 4;

        debug << " channel " << channel << " asic " << asicN << " fec " << fecN << endl;

        sampleCountRead = ntohs(pHeader.scnt);
        pay = sampleCountRead % 2;

        physChannel = -10;
        if (channel > 2 && channel < 15) {
            physChannel = channel - 3;
        } else if (channel > 15 && channel < 28) {
            physChannel = channel - 4;
        } else if (channel > 28 && channel < 53) {
            physChannel = channel - 5;
        } else if (channel > 53 && channel < 66) {
            physChannel = channel - 6;
        } else if (channel > 66) {
            physChannel = channel - 7;
        }

        if (physChannel >= 0)  // isThisAphysChannel?
                               // in this case we'd hold it in hEvent
                               // but we need to redefine the physChannel number
                               // so as not to overwrite the information
                               // and to know which ASIC and FEC it belongs to.
        {
            isData = true;
            physChannel = fecN * 72 * 4 + asicN * 72 + physChannel;

        } else
            isData = false;

        timeBin = 0;

        if (sampleCountRead < 9) isData = false;
        for (int i = 0; i < sampleCountRead; i++) {
            fread(&dat, sizeof(uint16_t), 1, fInputBinFile);
            frameBits += sizeof(dat);
            data = ntohs(dat);

              std::bitset<16> bs(data);
              debug<<bs<<endl;

            if (((data & 0xFE00) >> 9) == 8) {
                timeBin = GET_CELL_INDEX(data);
                if (timeBin == 511) isData = false;
                debug << data << " Time bin " << timeBin << endl;
            } else if ((((data & 0xF000) >> 12) == 0) && isData) {
                fSignalEvent->AddChargeToSignal(physChannel, timeBin, data);
                debug << "Time bin " << timeBin << " ADC: " << data << endl;
                timeBin++;
            }
        }

        debug << pay << endl;
        if (pay) {
            fread(&dat, sizeof(uint16_t), 1, fInputBinFile);
            frameBits += sizeof(uint16_t);
        }

        fread(&pEnd, sizeof(DataPacketEnd), 1, fInputBinFile);
        frameBits += sizeof(DataPacketEnd);

        debug << "Read "
             << sampleCountRead * sizeof(uint16_t) + sizeof(DataPacketHeader) +
                sizeof(DataPacketEnd) + sampleCountRead % 2 * sizeof(uint16_t)
             << " vs HeadSize " << ntohs(pHeader.size) << " Diff "
             << ntohs(pHeader.size) - (sampleCountRead + sizeof(DataPacketHeader) +
                sizeof(DataPacketEnd) + sampleCountRead % 2)
             << endl;
            debug << "Trailer_H " << ntohs(pEnd.crc1) << " Trailer_L " << ntohs(pEnd.crc2) << endl;
            debug << "Trailer " << eventTime << "\n" << endl;

    }  // end while
    totalBytesReaded += frameBits;

    // printf("Event ID %d time stored
    // %.3lf\n",fSignalEvent->GetID(),fSignalEvent->GetTime());

    debug << "End of event " << endl;

    return fSignalEvent;
}
