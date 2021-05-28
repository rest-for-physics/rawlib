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
// The TRestRawDreamToSignalProcess ...
//
// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
//
// \warning This process might be obsolete today. It may need additional
// revision, validation, and documentation. Use it under your own risk. If you
// find this process useful for your work feel free to use it, improve it,
// validate and/or document this process. If all those points are addressed
// these lines can be removed.
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
// 2019-May: First implementation
//           Damien Neyret
//
// 2021-May: Readapted to compile in REST v2.3.X
//           Damien Neyret
//
// \class      TRestRawDreamToSignalProcess
// \author     Damien Neyret
// \author     Javier Galan
//
// <hr>
/////
#include "TRestRawDreamToSignalProcess.h"
using namespace std;
#include "TTimeStamp.h"

ClassImp(TRestRawDreamToSignalProcess);

TRestRawDreamToSignalProcess::TRestRawDreamToSignalProcess() { Initialize(); }

TRestRawDreamToSignalProcess::TRestRawDreamToSignalProcess(char* cfgFileName)
    : TRestRawToSignalProcess(cfgFileName) {
    Initialize();
}

TRestRawDreamToSignalProcess::~TRestRawDreamToSignalProcess() {
    // TRestRawDreamToSignalProcess destructor
}

void TRestRawDreamToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    // MaxThreshold = 4000;
    bad_event = false;
    line = 0;                   // line number
    Nevent = 0, Nbadevent = 0;  // current event number
    IDEvent = 0;
}

void TRestRawDreamToSignalProcess::InitProcess() {
    tStart = 0;  // timeStamp of the run initially set to 0
    info << "TRestRawDreamToSignalProcess::InitProcess" << endl;

    totalBytesReaded = 0;
}

TRestEvent* TRestRawDreamToSignalProcess::ProcessEvent(TRestEvent* evInput) {
    FeuReadOut Feu;
    register bool badreadfg = false;

    debug << "---------------Start of TRestRawDreamToSignalProcess::ProcessEvent------------" << endl;

    fSignalEvent->Initialize();

    while (true) {  // loop on events

        Feu.NewEvent();  // reset Feu structure

        // Check header and fill Event IDs and TimeStamps
        badreadfg = ReadFeuHeaders(Feu);
        debug << "TRestRawDreamToSignalProcess::ProcessEvent: header read, badreadfg " << badreadfg << endl;
        debug << "TRestRawDreamToSignalProcess::ProcessEvent: event to read EventID " << Feu.EventID
              << " Time " << Feu.TimeStamp << " isample " << Feu.isample << endl;

        if (badreadfg) {
            warning << "TRestRawDreamToSignalProcess::ProcessEvent: Error in reading feu header (bad file or "
                       "end of file), trying to go to the next file"
                    << endl;
            if (GoToNextFile()) {
                badreadfg = ReadFeuHeaders(Feu);  // reading event from the next file
                debug << "TRestRawDreamToSignalProcess::ProcessEvent: header read, badreadfg " << badreadfg
                      << endl;
                debug << "TRestRawDreamToSignalProcess::ProcessEvent: event to read EventID " << Feu.EventID
                      << " Time " << Feu.TimeStamp << " isample " << Feu.isample << endl;
            } else {
                return NULL;
            }
        }

        // Read event
        badreadfg = ReadEvent(Feu);
        debug << "TRestRawDreamToSignalProcess::ProcessEvent: event read, badreadfg " << badreadfg << endl;
        if (badreadfg) {
            ferr << "TRestRawDreamToSignalProcess::ProcessEvent: Error in event reading at event " << Nevent
                 << endl;
            break;
        }

        Nevent++;
        if (badreadfg) break;  // break from loop
        if (bad_event) Nbadevent++;
        if ((Nevent % 100) == 0)
            cout << "TRestRawDreamToSignalProcess::ProcessEvent: " << Nevent
                 << " events processed in file, and " << Nbadevent << " bad events skipped " << endl;

        if (GetVerboseLevel() >= REST_Info) {
            info << "-- TRestRawDreamToSignalProcess::ProcessEvent ---" << endl;
            info << "Event ID : " << fSignalEvent->GetID() << endl;
            info << "Time stamp : " << fSignalEvent->GetTimeStamp() << endl;
            info << "Number of Signals : " << fSignalEvent->GetNumberOfSignals() << endl;
            info << "Number of Samples : " << (Feu.isample + 1) << endl;
            info << "-------------------------------------------------" << endl;
        }

        Feu.isample = -1;
        Feu.isample_prev = -2;
        Feu.NewEvent();
        bad_event = false;

        if (fSignalEvent->GetNumberOfSignals() == 0) {
            ferr << "TRestRawDreamToSignalProcess::ProcessEvent: no signal in event" << endl;
            return NULL;
        }

        debug << "TRestRawDreamToSignalProcess::ProcessEvent: returning signal event fSignalEvent "
              << fSignalEvent << endl;
        if (GetVerboseLevel() > REST_Debug) fSignalEvent->PrintEvent();
        return fSignalEvent;
    }

    return NULL;  // can't read data
}

//______________________________________________________________________________
//			Definition of decoding methods
bool TRestRawDreamToSignalProcess::ReadEvent(FeuReadOut& Feu) {
    register bool badreadfg = false;

    while (!Feu.event_completed) {
        badreadfg = ReadFeuHeaders(Feu);  // read feu header if not done
        debug << "TRestRawDreamToSignalProcess::ReadEvent: header read, badreadfg " << badreadfg << endl;
        if (badreadfg) {
            warning << "TRestRawDreamToSignalProcess::ReadEvent: error in reading FEU headers "
                    << endl;  // failed
            return true;
        }

        badreadfg = ReadDreamData(Feu);  // read dream data
        debug << "TRestRawDreamToSignalProcess::ReadEvent: data read, badreadfg " << badreadfg << endl;
        if (badreadfg) {
            ferr << "TRestRawDreamToSignalProcess::ReadEvent: error in reading Dream data " << endl;
            return true;
        }

        badreadfg = ReadFeuTrailer(Feu);  // read feu trailer
        debug << "TRestRawDreamToSignalProcess::ReadEvent: trailer read, badreadfg " << badreadfg << endl;
        if (badreadfg) {
            ferr << "TRestRawDreamToSignalProcess::ReadEvent: error in reading FEU trailer" << endl;
            return true;
        }

    }  // end loop
    info << "TRestRawDreamToSignalProcess::ReadEvent: Event ID " << Feu.EventID
         << " processed successfully, Time " << Feu.TimeStamp << " isample  " << Feu.isample << endl;

    return false;
}

bool TRestRawDreamToSignalProcess::ReadFeuHeaders(FeuReadOut& Feu) {
    if (Feu.FeuHeaderLoaded) return false;  // already done

    if (!Feu.data_to_treat) {  // data not loaded

        register int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            //       perror("TRestRawDreamToSignalProcess::ReadFeuHeaders: Error in reading FeuHeaders !");
            warning << "TRestRawDreamToSignalProcess::ReadFeuHeaders: Problem in reading raw file, ferror "
                    << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                    << fInputBinFile << endl;
            //      fclose(fInputBinFile);
            return true;  // failed
        }
        //  debug<<" Reading FeuHeaders ok, nbytes "<<nbytes<<endl;
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;
    }

    while (true) {  // loop on words of the header
        if (Feu.FeuHeaderLine < 8 && Feu.current_data.is_Feu_header()) {
            if (Feu.FeuHeaderLine == 0) {
                Feu.zs_mode = Feu.current_data.get_zs_mode();
                Feu.Id = Feu.current_data.get_Feu_ID();
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " ZS mode "
                      << Feu.zs_mode << " Id " << Feu.Id << endl;

            } else if (Feu.FeuHeaderLine == 1) {
                Feu.EventID = Feu.current_data.get_data();
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " EventID "
                      << Feu.EventID << endl;

            } else if (Feu.FeuHeaderLine == 2) {
                Feu.TimeStamp = Feu.current_data.get_data();
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp "
                      << Feu.TimeStamp << endl;

            } else if (Feu.FeuHeaderLine == 3) {
                Feu.FineTimeStamp = Feu.current_data.get_finetstp();
                Feu.isample_prev = Feu.isample;
                Feu.isample = Feu.current_data.get_sample_ID();
                // fprintf(stderr, "sampleIDprevious %d,  sampleID %d  \n", Feu.isample_prev,Feu.isample);
                Feu.FeuHeaderLoaded = true;
                if (Feu.isample != Feu.isample_prev + 1) {
                    if (Feu.isample_prev =
                            fMinPoints - 2) {  // finishing the current event and starting the next one
                                               // fprintf(stderr, "Event ID %d, processed \n", Feu.EventID-1);
                    } else {
                        ferr << "TRestRawDreamToSignalProcess::ReadFeuHeaders: non continuous sample index "
                                "number, isample = "
                             << Feu.isample << " prev_isample = " << Feu.isample_prev << endl;
                        bad_event = true;
                    }
                }
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " FineTimeStamp "
                      << Feu.FineTimeStamp << " isample " << Feu.isample << endl;

                // Reading optionals
            } else if (Feu.FeuHeaderLine == 4) {
                Feu.EventID_Op = Feu.current_data.get_data();
                Feu.EventID += (1 << 12) * Feu.EventID_Op;
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " EventID_Op "
                      << Feu.EventID_Op << endl;

            } else if (Feu.FeuHeaderLine == 5) {
                Feu.TimeStamp_Op1 = Feu.current_data.get_data();
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op1 "
                      << Feu.TimeStamp_Op1 << endl;

            } else if (Feu.FeuHeaderLine == 6) {
                Feu.TimeStamp_Op2 = Feu.current_data.get_data();
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op2 "
                      << Feu.TimeStamp_Op2 << endl;

            } else if (Feu.FeuHeaderLine == 7) {
                Feu.TimeStamp_Op3 = Feu.current_data.get_TimeStamp_Op();
                Feu.TimeStamp += ((long long)1 << 36) * Feu.TimeStamp_Op3 + (1 << 24) * Feu.TimeStamp_Op2 +
                                 (1 << 12) * Feu.TimeStamp_Op1;
                debug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op3 "
                      << Feu.TimeStamp_Op3 << " TimeStamp " << Feu.TimeStamp << endl;
            }

            Feu.data_to_treat = false;
            Feu.FeuHeaderLine++;

        } else if (Feu.FeuHeaderLine > 8 && Feu.current_data.is_Feu_header()) {
            ferr << "TRestRawDreamToSignalProcess::ReadFeuHeaders: too long Feu header part  "
                 << Feu.FeuHeaderLine << endl;
            bad_event = true;
        } else if (Feu.FeuHeaderLine > 3 && !Feu.current_data.is_Feu_header())
            break;  // header finished

        if (fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile) == 0) return true;
        totalBytesReaded += sizeof(Feu.current_data);
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;

    }  // end while

    fSignalEvent->SetID(Feu.EventID);
    fSignalEvent->SetTime(tStart + Feu.TimeStamp * 8.E-9);  // timeStamp in seconds

    return false;
}

bool TRestRawDreamToSignalProcess::ReadDreamData(FeuReadOut& Feu) {
    register bool got_raw_data_header = false;
    register bool got_channel_id = false;
    register int ichannel = 0;

    if (!Feu.FeuHeaderLoaded) {  // already loaded
        ferr << "TRestRawDreamToSignalProcess::ReadDreamData: error in ReadDreamData, Feu header not loaded"
             << endl;
        return true;
    }

    if (!Feu.data_to_treat) {  // no data to treat
        register int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            perror("TRestRawDreamToSignalProcess::ReadDreamData: no Dream data to read in file");
            ferr << "TRestRawDreamToSignalProcess::ReadDreamData:  problem in reading raw data file, ferror "
                 << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                 << fInputBinFile << endl;
            fclose(fInputBinFile);
            return true;  // failed
        }
        // debug<<" Reading DreamData ok, nbytes "<<nbytes<<endl;
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;
    }

    while (true) {  // loop on words in the Dream data main structure (not header or trailer ones)
        if (Feu.FeuHeaderLine > 3 && !Feu.current_data.is_Feu_header()) {
            if (Feu.DataHeaderLine < 4 && Feu.current_data.is_data_header()) {  // data header treatment
                if (Feu.DataHeaderLine == 0) {
                    Feu.TriggerID = Feu.current_data.get_data();  // trigger Id MSB
                    debug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                          << " TriggerID MSB " << Feu.TriggerID << endl;
                } else if (Feu.DataHeaderLine == 1) {
                    Feu.TriggerID_ISB = Feu.current_data.get_data();
                    debug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                          << " TriggerID_ISB " << Feu.TriggerID_ISB << endl;
                } else if (Feu.DataHeaderLine == 2) {
                    Feu.TriggerID_LSB = Feu.current_data.get_data();
                    Feu.TriggerID *= (1 << 24);
                    Feu.TriggerID += Feu.TriggerID_LSB + (1 << 12) * Feu.TriggerID_ISB;
                    debug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                          << " TriggerID_LSB " << Feu.TriggerID_LSB << " TriggerID " << Feu.TriggerID << endl;
                } else if (Feu.DataHeaderLine == 3) {
                    Feu.asicN = Feu.current_data.get_dream_ID();  // Dream_ID
                    ichannel = 0;  // reset counting of channels for the current Dream chip
                    Feu.channelN = 0;
                    Feu.DataTrailerLine = 0;
                    // fprintf(stderr, " asic N  %d \n", Feu.asicN);
                    got_raw_data_header = true;
                    debug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine << " asicN "
                          << Feu.asicN << endl;
                }
                Feu.DataHeaderLine++;
                Feu.data_to_treat = false;

            } else if (Feu.DataHeaderLine > 3 && Feu.current_data.is_data_header()) {
                bad_event = true;
                ferr << "TRestRawDreamToSignalProcess::ReadDreamData: too many data header lines, "
                        "DataHeaderLine "
                     << Feu.DataHeaderLine << endl;
                return true;

            } else if (Feu.current_data.is_data() &&
                       !Feu.zs_mode) {  // data lines treatment, non-zero suppression mode
                if (!got_raw_data_header) {
                    bad_event = true;
                    ferr << "TRestRawDreamToSignalProcess::ReadDreamData: data lines without header in "
                            "non-ZS mode "
                         << endl;
                }
                Feu.channelN = ichannel;
                if (!bad_event && Feu.channelN > -1 && Feu.channelN < NstripMax &&
                    Feu.isample < fMinPoints) {                              // fMinPoints=128
                    Feu.physChannel = Feu.asicN * NstripMax + Feu.channelN;  // channel's number on the DREAM

                    // loop on samples
                    if (Feu.physChannel < MaxPhysChannel) {
                        register Int_t sgnlIndex = fSignalEvent->GetSignalIndex(Feu.physChannel);
                        if (sgnlIndex == -1) {
                            sgnlIndex = fSignalEvent->GetNumberOfSignals();
                            TRestRawSignal sgnl(fMinPoints);
                            sgnl.SetSignalID(Feu.physChannel);
                            fSignalEvent->AddSignal(sgnl);
                        }
                        fSignalEvent->AddChargeToSignal(Feu.physChannel, Feu.isample,
                                                        Feu.current_data.get_data());
                    } else
                        ferr << "TRestRawDreamToSignalProcess::ReadDreamData: too large physical Channel in "
                                "non-ZS mode , Feu.physChannel=  "
                             << Feu.physChannel << " > MaxPhysChannel " << MaxPhysChannel << endl;
                    extreme << "ReadDreamData: nonZS physChannel " << Feu.physChannel << " get_data "
                            << Feu.current_data.get_data() << endl;
                }
                ichannel++;
                Feu.data_to_treat = false;

            } else if (Feu.current_data.is_data_zs() && Feu.zs_mode) {  // zero-suppression mode
                if (got_raw_data_header) {
                    bad_event = true;
                    ferr << "TRestRawDreamToSignalProcess::ReadDreamData: data lines with header in ZS mode "
                         << endl;
                }
                if (!got_channel_id && Feu.current_data.is_channel_ID()) {  // get channelID and dreamID
                    ichannel = Feu.current_data.get_channel_ID();
                    Feu.channelN = ichannel;
                    Feu.asicN = Feu.current_data.get_dream_ID();             // Dream_ID
                    Feu.physChannel = Feu.asicN * NstripMax + Feu.channelN;  // channel's number on the DREAM
                    got_channel_id = true;
                    if (Feu.channelN < 0 || Feu.channelN >= NstripMax) {
                        ferr << "TRestRawDreamToSignalProcess::ReadDreamData: too large channel number in ZS "
                                "mode , Feu.channelN=  "
                             << Feu.channelN << " > MaxPhysChannel " << MaxPhysChannel << endl;
                        bad_event = true;
                    }
                    extreme << "ReadDreamData: ZS header physChannel " << Feu.physChannel << " asicN "
                            << Feu.asicN << " ichannel " << ichannel << endl;
                } else {  // get channel data
                    got_channel_id = false;
                    if (!bad_event && Feu.channelN > -1 && Feu.channelN < NstripMax &&
                        Feu.isample < fMinPoints) {
                        Feu.channel_data = Feu.current_data.get_data();
                    }
                    extreme << "ReadDreamData: ZS data get_data " << Feu.current_data.get_data() << endl;
                }

                if (Feu.physChannel < MaxPhysChannel) {
                    register Int_t sgnlIndex = fSignalEvent->GetSignalIndex(Feu.physChannel);
                    if (sgnlIndex == -1) {
                        sgnlIndex = fSignalEvent->GetNumberOfSignals();
                        TRestRawSignal sgnl(fMinPoints);
                        sgnl.SetSignalID(Feu.physChannel);
                        fSignalEvent->AddSignal(sgnl);
                    }
                    fSignalEvent->AddChargeToSignal(Feu.physChannel, Feu.isample, Feu.channel_data);
                } else
                    ferr << "TRestRawDreamToSignalProcess::ReadDreamData: too large physical Channel in ZS "
                            "mode , Feu.physChannel=  "
                         << Feu.physChannel << " > MaxPhysChannel " << MaxPhysChannel << endl;
                Feu.data_to_treat = false;

            } else if (Feu.current_data.is_data_trailer()) {  // data trailer treatment

                if (ichannel != NstripMax && !Feu.zs_mode) {
                    bad_event = true;
                    ferr << "TRestRawDreamToSignalProcess::ReadDreamData: trailer with missing channel "
                            "numbers in non-ZS mode, ichannel "
                         << ichannel << endl;
                    return true;
                }
                if (got_channel_id && Feu.zs_mode) {
                    bad_event = true;
                    ferr << "TRestRawDreamToSignalProcess::ReadDreamData: trailer with channel Id without "
                            "channel data in ZS data, got_channel_id true"
                         << endl;
                    return true;
                }

                if (Feu.DataTrailerLine == 0) {  // CMN
                    Feu.CMN = Feu.current_data.get_data();
                    debug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine << " CMN "
                          << Feu.CMN << endl;
                } else if (Feu.DataTrailerLine == 1) {
                    Feu.CMN_rest = Feu.current_data.get_data();
                    debug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine << " CMN_rest "
                          << Feu.CMN_rest << endl;
                } else if (Feu.DataTrailerLine == 2) {  // Cell_ID
                    Feu.Cell_ID_MSB = Feu.current_data.get_data();
                    debug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                          << " Cell_ID_MSB " << Feu.Cell_ID_MSB << endl;
                } else if (Feu.DataTrailerLine == 3) {
                    Feu.Cell_ID_ISB = Feu.current_data.get_data();
                    debug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                          << " Cell_ID_ISB " << Feu.Cell_ID_ISB << endl;
                } else if (Feu.DataTrailerLine == 4) {
                    Feu.Cell_ID_LSB = Feu.current_data.get_data();
                    Feu.Cell_ID = Feu.Cell_ID_LSB + (1 << 12) * Feu.Cell_ID_ISB +
                                  ((long long)1 << 24) * Feu.Cell_ID_MSB;
                    debug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                          << " Cell_ID_LSB " << Feu.Cell_ID_LSB << " Cell_ID " << Feu.Cell_ID << endl;
                    Feu.DataHeaderLine = 0;
                    got_raw_data_header = false;
                    Feu.channelN = 0;
                }
                Feu.DataTrailerLine++;
                Feu.data_to_treat = false;

            } else if (Feu.DataTrailerLine > 4 && Feu.current_data.is_data_trailer()) {
                bad_event = true;
                ferr << "TRestRawDreamToSignalProcess::ReadDreamData: too many data trailer lines, "
                        "DataTrailerLine "
                     << Feu.DataTrailerLine << endl;
                return true;

            } else if (Feu.current_data.is_final_trailer())
                break;  // Dream raw data finished
        }

        if (fread((char*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile) == 0) return true;
        totalBytesReaded += sizeof(Feu.current_data);
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;

    }  // end while
    return false;
}

bool TRestRawDreamToSignalProcess::ReadFeuTrailer(FeuReadOut& Feu) {
    if (!Feu.data_to_treat) {
        register int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            perror("TRestRawDreamToSignalProcess::ReadFeuTrailer: can't read new data from file");
            ferr << "TRestRawDreamToSignalProcess::ReadFeuTrailer: can't read new data from file, ferror "
                 << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                 << fInputBinFile << endl;
            fclose(fInputBinFile);
            return true;  // failed
        }
        debug << "TRestRawDreamToSignalProcess::ReadFeuTrailer: Reading FeuTrailer ok, nbytes " << nbytes
              << endl;
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;
    }

    while (true) {
        if (Feu.current_data.is_final_trailer()) {
            if (Feu.channelN != 0) {
                bad_event = true;
                ferr << "TRestRawDreamToSignalProcess::ReadFeuTrailer: channel number not null in trailer, "
                        "Feu.channelN "
                     << Feu.channelN << endl;
                return true;
            }
            if (Feu.current_data.is_end_of_event()) {
                if (Feu.isample != (fMinPoints - 1)) {
                    warning << "TRestRawDreamToSignalProcess::ReadFeuTrailer: not all samples read at end of "
                               "event, isample "
                            << Feu.isample << " MinPoints " << fMinPoints << endl;
                }
                //         Feu.isample=-1; Feu.isample_prev=-2;
                Feu.event_completed = true;
            }

            Feu.FeuHeaderLine = 0;
            Feu.FeuHeaderLoaded = false;
            Feu.zs_mode = false;
            Feu.data_to_treat = false;

            // Reading VEP, not used
            fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
            totalBytesReaded += sizeof(Feu.current_data);
            break;
        }

        if (fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile) == 0) return true;
        totalBytesReaded += sizeof(Feu.current_data);
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;

    }  // end while
    return false;
}

