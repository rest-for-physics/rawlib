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
// The TRestRawFEUDreamToSignalProcess ...
//
// DOCUMENTATION TO BE WRITTEN (main description, methods, data members)
//
// TODO: This process requires optimization to improve the data processing
// rate.
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
// \class      TRestRawFEUDreamToSignalProcess
// \author     Damien Neyret
// \author     Javier Galan
//
// <hr>
/////
#include "TRestRawFEUDreamToSignalProcess.h"

using namespace std;

#include "TTimeStamp.h"

ClassImp(TRestRawFEUDreamToSignalProcess);

TRestRawFEUDreamToSignalProcess::TRestRawFEUDreamToSignalProcess() { Initialize(); }

TRestRawFEUDreamToSignalProcess::TRestRawFEUDreamToSignalProcess(const char* configFilename)
    : TRestRawToSignalProcess(configFilename) {
    Initialize();
}

TRestRawFEUDreamToSignalProcess::~TRestRawFEUDreamToSignalProcess() {
    // TRestRawFEUDreamToSignalProcess destructor
}

void TRestRawFEUDreamToSignalProcess::Initialize() {
    TRestRawToSignalProcess::Initialize();

    // MaxThreshold = 4000;
    bad_event = false;
    line = 0;                   // line number
    Nevent = 0, Nbadevent = 0;  // current event number
    IDEvent = 0;
}

void TRestRawFEUDreamToSignalProcess::InitProcess() {
    tStart = 0;  // timeStamp of the run initially set to 0
    RESTInfo << "TRestRawFEUDreamToSignalProcess::InitProcess" << RESTendl;

    totalBytesReaded = 0;
}

TRestEvent* TRestRawFEUDreamToSignalProcess::ProcessEvent(TRestEvent* inputEvent) {
    FeuReadOut Feu;
    bool badreadfg = false;

    RESTDebug << "---------------Start of TRestRawFEUDreamToSignalProcess::ProcessEvent------------"
              << RESTendl;

    fSignalEvent->Initialize();

    while (true) {  // loop on events

        Feu.NewEvent();  // reset Feu structure

        // Check header and fill Event IDs and TimeStamps
        badreadfg = ReadFeuHeaders(Feu);
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: header read, badreadfg " << badreadfg
                  << RESTendl;
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: event to read EventID " << Feu.EventID
                  << " Time " << Feu.TimeStamp << " isample " << Feu.isample << RESTendl;

        if (badreadfg) {
            RESTWarning
                << "TRestRawFEUDreamToSignalProcess::ProcessEvent: Error in reading feu header (bad file or "
                   "end of file), trying to go to the next file"
                << RESTendl;
            if (GoToNextFile()) {
                badreadfg = ReadFeuHeaders(Feu);  // reading event from the next file
                RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: header read, badreadfg "
                          << badreadfg << RESTendl;
                RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: event to read EventID "
                          << Feu.EventID << " Time " << Feu.TimeStamp << " isample " << Feu.isample
                          << RESTendl;
            } else {
                return nullptr;
            }
        }

        // Read event
        badreadfg = ReadEvent(Feu);
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: event read, badreadfg " << badreadfg
                  << RESTendl;
        if (badreadfg) {
            RESTError << "TRestRawFEUDreamToSignalProcess::ProcessEvent: Error in event reading at event "
                      << Nevent << RESTendl;
            break;
        }

        Nevent++;
        if (badreadfg) break;  // break from loop
        if (bad_event) Nbadevent++;
        if ((Nevent % 100) == 0)
            cout << "TRestRawFEUDreamToSignalProcess::ProcessEvent: " << Nevent
                 << " events processed in file, and " << Nbadevent << " bad events skipped " << endl;

        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Info) {
            RESTInfo << "-- TRestRawFEUDreamToSignalProcess::ProcessEvent ---" << RESTendl;
            RESTInfo << "Event ID : " << fSignalEvent->GetID() << RESTendl;
            RESTInfo << "Time stamp : " << fSignalEvent->GetTimeStamp() << RESTendl;
            RESTInfo << "Number of Signals : " << fSignalEvent->GetNumberOfSignals() << RESTendl;
            RESTInfo << "Number of Samples : " << (Feu.isample + 1) << RESTendl;
            RESTInfo << "-------------------------------------------------" << RESTendl;
        }

        Feu.isample = -1;
        Feu.isample_prev = -2;
        Feu.NewEvent();
        bad_event = false;

        if (fSignalEvent->GetNumberOfSignals() == 0) {
            RESTError << "TRestRawFEUDreamToSignalProcess::ProcessEvent: no signal in event" << RESTendl;
            return nullptr;
        }

        RESTDebug << "TRestRawFEUDreamToSignalProcess::ProcessEvent: returning signal event fSignalEvent "
                  << fSignalEvent << RESTendl;
        if (GetVerboseLevel() > TRestStringOutput::REST_Verbose_Level::REST_Debug) fSignalEvent->PrintEvent();
        return fSignalEvent;
    }

    return nullptr;  // can't read data
}

//			Definition of decoding methods
bool TRestRawFEUDreamToSignalProcess::ReadEvent(FeuReadOut& Feu) {
    bool badreadfg = false;

    while (!Feu.event_completed) {
        badreadfg = ReadFeuHeaders(Feu);  // read feu header if not done
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ReadEvent: header read, badreadfg " << badreadfg
                  << RESTendl;
        if (badreadfg) {
            RESTWarning << "TRestRawFEUDreamToSignalProcess::ReadEvent: error in reading FEU headers "
                        << RESTendl;  // failed
            return true;
        }

        badreadfg = ReadDreamData(Feu);  // read dream data
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ReadEvent: data read, badreadfg " << badreadfg
                  << RESTendl;
        if (badreadfg) {
            RESTError << "TRestRawFEUDreamToSignalProcess::ReadEvent: error in reading Dream data "
                      << RESTendl;
            return true;
        }

        badreadfg = ReadFeuTrailer(Feu);  // read feu trailer
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ReadEvent: trailer read, badreadfg " << badreadfg
                  << RESTendl;
        if (badreadfg) {
            RESTError << "TRestRawFEUDreamToSignalProcess::ReadEvent: error in reading FEU trailer"
                      << RESTendl;
            return true;
        }

    }  // end loop
    RESTInfo << "TRestRawFEUDreamToSignalProcess::ReadEvent: Event ID " << Feu.EventID
             << " processed successfully, Time " << Feu.TimeStamp << " isample  " << Feu.isample << RESTendl;

    return false;
}

bool TRestRawFEUDreamToSignalProcess::ReadFeuHeaders(FeuReadOut& Feu) {
    if (Feu.FeuHeaderLoaded) return false;  // already done

    if (!Feu.data_to_treat) {  // data not loaded

        int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            //       perror("TRestRawFEUDreamToSignalProcess::ReadFeuHeaders: Error in reading FeuHeaders !");
            RESTWarning
                << "TRestRawFEUDreamToSignalProcess::ReadFeuHeaders: Problem in reading raw file, ferror "
                << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                << fInputBinFile << RESTendl;
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
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " ZS mode "
                          << Feu.zs_mode << " Id " << Feu.Id << RESTendl;

            } else if (Feu.FeuHeaderLine == 1) {
                Feu.EventID = Feu.current_data.get_data();
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " EventID "
                          << Feu.EventID << RESTendl;

            } else if (Feu.FeuHeaderLine == 2) {
                Feu.TimeStamp = Feu.current_data.get_data();
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp "
                          << Feu.TimeStamp << RESTendl;

            } else if (Feu.FeuHeaderLine == 3) {
                Feu.FineTimeStamp = Feu.current_data.get_finetstp();
                Feu.isample_prev = Feu.isample;
                Feu.isample = Feu.current_data.get_sample_ID();
                // fprintf(stderr, "sampleIDprevious %d,  sampleID %d  \n", Feu.isample_prev,Feu.isample);
                Feu.FeuHeaderLoaded = true;
                if (Feu.isample != Feu.isample_prev + 1) {
                    if (Feu.isample_prev ==
                        fMinPoints - 2) {  // finishing the current event and starting the next one
                                           // fprintf(stderr, "Event ID %d, processed \n", Feu.EventID-1);
                    } else {
                        RESTError
                            << "TRestRawFEUDreamToSignalProcess::ReadFeuHeaders: non continuous sample index "
                               "number, isample = "
                            << Feu.isample << " prev_isample = " << Feu.isample_prev << RESTendl;
                        bad_event = true;
                    }
                }
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " FineTimeStamp "
                          << Feu.FineTimeStamp << " isample " << Feu.isample << RESTendl;

                // Reading optionals
            } else if (Feu.FeuHeaderLine == 4) {
                Feu.EventID_Op = Feu.current_data.get_data();
                Feu.EventID += (1 << 12) * Feu.EventID_Op;
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " EventID_Op "
                          << Feu.EventID_Op << RESTendl;

            } else if (Feu.FeuHeaderLine == 5) {
                Feu.TimeStamp_Op1 = Feu.current_data.get_data();
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op1 "
                          << Feu.TimeStamp_Op1 << RESTendl;

            } else if (Feu.FeuHeaderLine == 6) {
                Feu.TimeStamp_Op2 = Feu.current_data.get_data();
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op2 "
                          << Feu.TimeStamp_Op2 << RESTendl;

            } else if (Feu.FeuHeaderLine == 7) {
                Feu.TimeStamp_Op3 = Feu.current_data.get_TimeStamp_Op();
                Feu.TimeStamp += ((long long)1 << 36) * Feu.TimeStamp_Op3 + (1 << 24) * Feu.TimeStamp_Op2 +
                                 (1 << 12) * Feu.TimeStamp_Op1;
                RESTDebug << "ReadFeuHeaders: header FeuHeaderLine " << Feu.FeuHeaderLine << " TimeStamp_Op3 "
                          << Feu.TimeStamp_Op3 << " TimeStamp " << Feu.TimeStamp << RESTendl;
            }

            Feu.data_to_treat = false;
            Feu.FeuHeaderLine++;

        } else if (Feu.FeuHeaderLine > 8 && Feu.current_data.is_Feu_header()) {
            RESTError << "TRestRawFEUDreamToSignalProcess::ReadFeuHeaders: too long Feu header part  "
                      << Feu.FeuHeaderLine << RESTendl;
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

bool TRestRawFEUDreamToSignalProcess::ReadDreamData(FeuReadOut& Feu) {
    bool got_raw_data_header = false;
    bool got_channel_id = false;
    int ichannel = 0;

    if (!Feu.FeuHeaderLoaded) {  // already loaded
        RESTError
            << "TRestRawFEUDreamToSignalProcess::ReadDreamData: error in ReadDreamData, Feu header not loaded"
            << RESTendl;
        return true;
    }

    if (!Feu.data_to_treat) {  // no data to treat
        int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            perror("TRestRawFEUDreamToSignalProcess::ReadDreamData: no Dream data to read in file");
            RESTError << "TRestRawFEUDreamToSignalProcess::ReadDreamData:  problem in reading raw data file, "
                         "ferror "
                      << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                      << fInputBinFile << RESTendl;
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
                    RESTDebug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                              << " TriggerID MSB " << Feu.TriggerID << RESTendl;
                } else if (Feu.DataHeaderLine == 1) {
                    Feu.TriggerID_ISB = Feu.current_data.get_data();
                    RESTDebug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                              << " TriggerID_ISB " << Feu.TriggerID_ISB << RESTendl;
                } else if (Feu.DataHeaderLine == 2) {
                    Feu.TriggerID_LSB = Feu.current_data.get_data();
                    Feu.TriggerID *= (1 << 24);
                    Feu.TriggerID += Feu.TriggerID_LSB + (1 << 12) * Feu.TriggerID_ISB;
                    RESTDebug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine
                              << " TriggerID_LSB " << Feu.TriggerID_LSB << " TriggerID " << Feu.TriggerID
                              << RESTendl;
                } else if (Feu.DataHeaderLine == 3) {
                    Feu.asicN = Feu.current_data.get_dream_ID();  // Dream_ID
                    ichannel = 0;  // reset counting of channels for the current Dream chip
                    Feu.channelN = 0;
                    Feu.DataTrailerLine = 0;
                    // fprintf(stderr, " asic N  %d \n", Feu.asicN);
                    got_raw_data_header = true;
                    RESTDebug << "ReadDreamData: header DataHeaderLine " << Feu.DataHeaderLine << " asicN "
                              << Feu.asicN << RESTendl;
                }
                Feu.DataHeaderLine++;
                Feu.data_to_treat = false;

            } else if (Feu.DataHeaderLine > 3 && Feu.current_data.is_data_header()) {
                bad_event = true;
                RESTError << "TRestRawFEUDreamToSignalProcess::ReadDreamData: too many data header lines, "
                             "DataHeaderLine "
                          << Feu.DataHeaderLine << RESTendl;
                return true;

            } else if (Feu.current_data.is_data() &&
                       !Feu.zs_mode) {  // data lines treatment, non-zero suppression mode
                if (!got_raw_data_header) {
                    bad_event = true;
                    RESTError
                        << "TRestRawFEUDreamToSignalProcess::ReadDreamData: data lines without header in "
                           "non-ZS mode "
                        << RESTendl;
                }
                Feu.channelN = ichannel;
                if (!bad_event && Feu.channelN > -1 && Feu.channelN < NstripMax &&
                    Feu.isample < fMinPoints) {                              // fMinPoints=128
                    Feu.physChannel = Feu.asicN * NstripMax + Feu.channelN;  // channel's number on the DREAM

                    // loop on samples
                    if (Feu.physChannel < MaxPhysChannel) {
                        Int_t signalIndex = fSignalEvent->GetSignalIndex(Feu.physChannel);
                        if (signalIndex == -1) {
                            signalIndex = fSignalEvent->GetNumberOfSignals();
                            TRestRawSignal signal(fMinPoints);
                            signal.SetSignalID(Feu.physChannel);
                            fSignalEvent->AddSignal(signal);
                        }
                        fSignalEvent->AddChargeToSignal(Feu.physChannel, Feu.isample,
                                                        Feu.current_data.get_data());
                    } else
                        RESTError
                            << "TRestRawFEUDreamToSignalProcess::ReadDreamData: too large physical Channel "
                               "in "
                               "non-ZS mode , Feu.physChannel=  "
                            << Feu.physChannel << " > MaxPhysChannel " << MaxPhysChannel << RESTendl;
                    RESTExtreme << "ReadDreamData: nonZS physChannel " << Feu.physChannel << " get_data "
                                << Feu.current_data.get_data() << RESTendl;
                }
                ichannel++;
                Feu.data_to_treat = false;

            } else if (Feu.current_data.is_data_zs() && Feu.zs_mode) {  // zero-suppression mode
                if (got_raw_data_header) {
                    bad_event = true;
                    RESTError
                        << "TRestRawFEUDreamToSignalProcess::ReadDreamData: data lines with header in ZS "
                           "mode "
                        << RESTendl;
                }
                if (!got_channel_id && Feu.current_data.is_channel_ID()) {  // get channelID and dreamID
                    ichannel = Feu.current_data.get_channel_ID();
                    Feu.channelN = ichannel;
                    Feu.asicN = Feu.current_data.get_dream_ID();             // Dream_ID
                    Feu.physChannel = Feu.asicN * NstripMax + Feu.channelN;  // channel's number on the DREAM
                    got_channel_id = true;
                    if (Feu.channelN < 0 || Feu.channelN >= NstripMax) {
                        RESTError
                            << "TRestRawFEUDreamToSignalProcess::ReadDreamData: too large channel number in "
                               "ZS "
                               "mode , Feu.channelN=  "
                            << Feu.channelN << " > MaxPhysChannel " << MaxPhysChannel << RESTendl;
                        bad_event = true;
                    }
                    RESTExtreme << "ReadDreamData: ZS header physChannel " << Feu.physChannel << " asicN "
                                << Feu.asicN << " ichannel " << ichannel << RESTendl;
                } else {  // get channel data
                    got_channel_id = false;
                    if (!bad_event && Feu.channelN > -1 && Feu.channelN < NstripMax &&
                        Feu.isample < fMinPoints) {
                        Feu.channel_data = Feu.current_data.get_data();
                    }
                    RESTExtreme << "ReadDreamData: ZS data get_data " << Feu.current_data.get_data()
                                << RESTendl;
                }

                if (Feu.physChannel < MaxPhysChannel) {
                    Int_t signalIndex = fSignalEvent->GetSignalIndex(Feu.physChannel);
                    if (signalIndex == -1) {
                        signalIndex = fSignalEvent->GetNumberOfSignals();
                        TRestRawSignal signal(fMinPoints);
                        signal.SetSignalID(Feu.physChannel);
                        fSignalEvent->AddSignal(signal);
                    }
                    fSignalEvent->AddChargeToSignal(Feu.physChannel, Feu.isample, Feu.channel_data);
                } else
                    RESTError
                        << "TRestRawFEUDreamToSignalProcess::ReadDreamData: too large physical Channel in ZS "
                           "mode , Feu.physChannel=  "
                        << Feu.physChannel << " > MaxPhysChannel " << MaxPhysChannel << RESTendl;
                Feu.data_to_treat = false;

            } else if (Feu.current_data.is_data_trailer()) {  // data trailer treatment

                if (ichannel != NstripMax && !Feu.zs_mode) {
                    bad_event = true;
                    RESTError
                        << "TRestRawFEUDreamToSignalProcess::ReadDreamData: trailer with missing channel "
                           "numbers in non-ZS mode, ichannel "
                        << ichannel << RESTendl;
                    return true;
                }
                if (got_channel_id && Feu.zs_mode) {
                    bad_event = true;
                    RESTError
                        << "TRestRawFEUDreamToSignalProcess::ReadDreamData: trailer with channel Id without "
                           "channel data in ZS data, got_channel_id true"
                        << RESTendl;
                    return true;
                }

                if (Feu.DataTrailerLine == 0) {  // CMN
                    Feu.CMN = Feu.current_data.get_data();
                    RESTDebug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine << " CMN "
                              << Feu.CMN << RESTendl;
                } else if (Feu.DataTrailerLine == 1) {
                    Feu.CMN_rest = Feu.current_data.get_data();
                    RESTDebug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                              << " CMN_rest " << Feu.CMN_rest << RESTendl;
                } else if (Feu.DataTrailerLine == 2) {  // Cell_ID
                    Feu.Cell_ID_MSB = Feu.current_data.get_data();
                    RESTDebug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                              << " Cell_ID_MSB " << Feu.Cell_ID_MSB << RESTendl;
                } else if (Feu.DataTrailerLine == 3) {
                    Feu.Cell_ID_ISB = Feu.current_data.get_data();
                    RESTDebug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                              << " Cell_ID_ISB " << Feu.Cell_ID_ISB << RESTendl;
                } else if (Feu.DataTrailerLine == 4) {
                    Feu.Cell_ID_LSB = Feu.current_data.get_data();
                    Feu.Cell_ID = Feu.Cell_ID_LSB + (1 << 12) * Feu.Cell_ID_ISB +
                                  ((long long)1 << 24) * Feu.Cell_ID_MSB;
                    RESTDebug << "ReadDreamData: trailer DataTrailerLine " << Feu.DataTrailerLine
                              << " Cell_ID_LSB " << Feu.Cell_ID_LSB << " Cell_ID " << Feu.Cell_ID << RESTendl;
                    Feu.DataHeaderLine = 0;
                    got_raw_data_header = false;
                    Feu.channelN = 0;
                }
                Feu.DataTrailerLine++;
                Feu.data_to_treat = false;

            } else if (Feu.DataTrailerLine > 4 && Feu.current_data.is_data_trailer()) {
                bad_event = true;
                RESTError << "TRestRawFEUDreamToSignalProcess::ReadDreamData: too many data trailer lines, "
                             "DataTrailerLine "
                          << Feu.DataTrailerLine << RESTendl;
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

bool TRestRawFEUDreamToSignalProcess::ReadFeuTrailer(FeuReadOut& Feu) {
    if (!Feu.data_to_treat) {
        int nbytes = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
        totalBytesReaded += sizeof(Feu.current_data);
        if (nbytes == 0) {
            perror("TRestRawFEUDreamToSignalProcess::ReadFeuTrailer: can't read new data from file");
            RESTError
                << "TRestRawFEUDreamToSignalProcess::ReadFeuTrailer: can't read new data from file, ferror "
                << ferror(fInputBinFile) << " feof " << feof(fInputBinFile) << " fInputBinFile "
                << fInputBinFile << RESTendl;
            fclose(fInputBinFile);
            return true;  // failed
        }
        RESTDebug << "TRestRawFEUDreamToSignalProcess::ReadFeuTrailer: Reading FeuTrailer ok, nbytes "
                  << nbytes << RESTendl;
        Feu.current_data.ntohs_();
        Feu.data_to_treat = true;
    }

    while (true) {
        if (Feu.current_data.is_final_trailer()) {
            if (Feu.channelN != 0) {
                bad_event = true;
                RESTError << "TRestRawFEUDreamToSignalProcess::ReadFeuTrailer: channel number not NULL in "
                             "trailer, "
                             "Feu.channelN "
                          << Feu.channelN << RESTendl;
                return true;
            }
            if (Feu.current_data.is_end_of_event()) {
                if (Feu.isample != (fMinPoints - 1)) {
                    RESTWarning
                        << "TRestRawFEUDreamToSignalProcess::ReadFeuTrailer: not all samples read at end of "
                           "event, isample "
                        << Feu.isample << " MinPoints " << fMinPoints << RESTendl;
                }
                //         Feu.isample=-1; Feu.isample_prev=-2;
                Feu.event_completed = true;
            }

            Feu.FeuHeaderLine = 0;
            Feu.FeuHeaderLoaded = false;
            Feu.zs_mode = false;
            Feu.data_to_treat = false;

            // Reading VEP, not used
            int z = fread((void*)&(Feu.current_data), sizeof(Feu.current_data), 1, fInputBinFile);
            if (z == 0)
                RESTError << "TRestRawFEUDreamToSignalProcess::ReadFeuTrailer. Error reading file"
                          << RESTendl;
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
