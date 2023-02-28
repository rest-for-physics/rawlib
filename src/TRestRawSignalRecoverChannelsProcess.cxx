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
/// The TRestRawSignalRecoverChannelsProcess allows to recover a selection
/// of daq channel ids from a TRestRawSignalEvent. The dead channels must
/// be known beforehand and the signal ids to be recovered must be
/// specified through the corresponding section at the RML configuration
/// file.
///
/// The following example will apply the recovery algorithm for the
/// channels with signal ids 17,19,27 and 67. The signal ids must exist
/// in the readout defined through the TRestDetectorReadout structure.
///
/// \code
/// <TRestRawSignalRecoverChannelsProcess name="returnChannels"
/// title="Recovering few channels" verboseLevel="debug" >
///     <parameter name="channelIds" value="{17,27,67}" />
/// </TRestRawSignalRecoverChannelsProcess>
/// \endcode
///
/// The dead channel reconstruction algorithm is for the moment very
/// simple. The charge of the dead channel is directly calculated by
/// using the charge of the adjacent readout channels,
/// \f$s_i = 0.5 \times (s_{i-1} + s_{i+1})\f$
///
/// This process will access the information of the decoding stored in
/// the TRestDetectorReadout definition to assure that the righ signal ids,
/// corresponding to the adjacent channels, are used in the calculation.
///
/// \warning This process will only be functional if the detectorlib
/// was compiled. You may check if it is the case executing the command
/// `rest-config --libs`, and checking the output shows `-lRestDetector`.
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2017-November: First implementation of TRestRawSignalRecoverChannelsProcess.
///             Javier Galan
///
/// \class      TRestRawSignalRecoverChannelsProcess
/// \author     Javier Galan
///
/// <hr>
///
#include "TRestRawSignalRecoverChannelsProcess.h"

#include <TRandom3.h>

using namespace std;

ClassImp(TRestRawSignalRecoverChannelsProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalRecoverChannelsProcess::TRestRawSignalRecoverChannelsProcess() { Initialize(); }

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
/// \param configFilename A const char* giving the path to an RML file.
///
TRestRawSignalRecoverChannelsProcess::TRestRawSignalRecoverChannelsProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename) == -1) LoadDefaultConfig();

    PrintMetadata();
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalRecoverChannelsProcess::~TRestRawSignalRecoverChannelsProcess() { delete fOutputSignalEvent; }

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawSignalRecoverChannelsProcess::LoadDefaultConfig() {
    SetName("removeChannels-Default");
    SetTitle("Default config");
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalRecoverChannelsProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputSignalEvent = nullptr;
    fOutputSignalEvent = new TRestRawSignalEvent();
}

///////////////////////////////////////////////
/// \brief Function to load the configuration from an external configuration
/// file.
///
/// If no configuration path is defined in TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// \param configFilename A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the
/// correspondig TRestGeant4AnalysisProcess section inside the RML.
///
void TRestRawSignalRecoverChannelsProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name) == -1) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Function to initialize the process.
/// TRestRawSignalRecoverChannelsProcess requires to get a pointer to
/// TRestDetectorReadout.
///
void TRestRawSignalRecoverChannelsProcess::InitProcess() {
#ifdef REST_DetectorLib
    fReadout = GetMetadata<TRestDetectorReadout>();

    if (fReadout == nullptr) {
        cout << "REST ERROR: Readout has not been initialized" << endl;
        exit(-1);
    }
#else
    RESTError << "TRestRawSignalRecoverChannelsProcess will not be active." << RESTendl;
    RESTError << "REST was not compiled with detectorlib" << RESTendl;
    RESTError << "Please, remove this process or compile REST with detector library" << RESTendl;
#endif
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalRecoverChannelsProcess::ProcessEvent(TRestEvent* evInput) {
    fInputSignalEvent = (TRestRawSignalEvent*)evInput;

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++)
        fOutputSignalEvent->AddSignal(*fInputSignalEvent->GetSignal(n));

    Int_t nPoints = fOutputSignalEvent->GetSignal(0)->GetNumberOfPoints();

    Int_t idL;
    Int_t idR;
    Int_t idL2;
    Int_t idR2;
    for (unsigned int x = 0; x < fChannelIds.size(); x++) {
        GetAdjacentSignalIds(fChannelIds[x], idL, idR, idL2, idR2);
        // cout << "Channel id : " << fChannelIds[x] << " Left : " << idL << " Right : " << idR << endl;

        if (fOutputSignalEvent->GetSignalIndex(fChannelIds[x]) > 0)
            fOutputSignalEvent->RemoveSignalWithId(fChannelIds[x]);

        if (idL == -1 || idR == -1 || idL2 == -1 || idR2 == -1) continue;

        TRestRawSignal* leftSgnl = fInputSignalEvent->GetSignalById(idL);
        TRestRawSignal* rightSgnl = fInputSignalEvent->GetSignalById(idR);
        TRestRawSignal* leftSgnl2 = fInputSignalEvent->GetSignalById(idL2);
        TRestRawSignal* rightSgnl2 = fInputSignalEvent->GetSignalById(idR2);

        TRandom3* r = new TRandom3();
        r->SetSeed(0);

        if (leftSgnl == nullptr && rightSgnl == nullptr) continue;

        // For organize a bit more. a and b give information about the neighbours and whether they are death
        // or not
        int a = 0, b = 0;
        if (fChannelIds[x] == (fChannelIds[x + 1] - 1)) {
            a = 1;
        }
        if (x > 0 && fChannelIds[x] == (fChannelIds[x - 1] + 1)) {
            b = 1;
        }

        if (a == 0 && b == 0) {  // if the neighbours aren't dead and one of them doesn't have signal, we
                                 // chose to take it into account or not
            if (leftSgnl == nullptr || rightSgnl == nullptr) {
                double_t choice = r->Rndm();
                if (choice < 0.5) continue;
            }
        }

        TRestRawSignal* recoveredSignal = new TRestRawSignal();
        recoveredSignal->SetID(fChannelIds[x]);

        vector<Short_t> dataRecovered(nPoints);
        vector<Short_t> dataRight(nPoints);
        vector<Short_t> dataLeft(nPoints);
        vector<Short_t> dataRightClean(nPoints);
        vector<Short_t> dataLeftClean(nPoints);
        for (int n = 0; n < nPoints; n++) {
            dataRecovered[n] = 0;
            dataRight[n] = 0;
            dataLeft[n] = 0;
            dataRightClean[n] = 0;
            dataLeftClean[n] = 0;
        }

        if (a == 0 && b == 0) {  // if the neigbours are alive
            if (leftSgnl != nullptr) {
                for (int n = 0; n < nPoints; n++) {
                    dataRecovered[n] = leftSgnl->GetRawData(n);
                    dataLeft[n] = leftSgnl->GetRawData(n);
                    dataLeftClean[n] = leftSgnl->GetData(n);
                }
            }

            if (rightSgnl != nullptr) {
                for (int n = 0; n < nPoints; n++) {
                    dataRecovered[n] += rightSgnl->GetRawData(n);
                    dataRight[n] = rightSgnl->GetRawData(n);
                    dataRightClean[n] = rightSgnl->GetData(n);
                }
            }
        }
        /*else{
            if (leftSgnl2 != nullptr) {
                for (int n = 0; n < nPoints; n++) dataRecovered[n] = leftSgnl2->GetData(n);
            }
            if (rightSgnl2 != nullptr) {
                for (int n = 0; n < nPoints; n++) dataRecovered[n] += rightSgnl2->GetData(n);
            }
        }*/ //This part would be for the case where two dead channels are together, then the recovered channel will be the average between the second-neighbours

        vector<Short_t> offSet(nPoints);
        for (int n = 0; n < nPoints; n++)
            offSet[n] = dataRight[n] - dataRightClean[n];  // In IAXO-D0 both values are the same

        if (a == 0 && b == 0) {
            for (int n = 0; n < nPoints; n++) recoveredSignal->AddPoint(dataRecovered[n] / 2.);
        } else {
            for (int n = 0; n < nPoints; n++) recoveredSignal->AddPoint(dataRecovered[n] / 2.);
        }

        fOutputSignalEvent->AddSignal(*recoveredSignal);

        TRestRawSignal* newSignalR = new TRestRawSignal();
        newSignalR->SetID(idR);
        TRestRawSignal* newSignalL = new TRestRawSignal();
        newSignalL->SetID(idL);
        if (fOutputSignalEvent->GetSignalIndex(idR) > 0) fOutputSignalEvent->RemoveSignalWithId(idR);
        if (fOutputSignalEvent->GetSignalIndex(idL) > 0) fOutputSignalEvent->RemoveSignalWithId(idL);

        double_t prop = 0.;
        Short_t aux;

        // Subtraction of the added signal from the neighbours, proportional to the signal in each one (to
        // conserve the total energy of the event)
        if (rightSgnl != nullptr && leftSgnl != nullptr) {
            for (int n = 0; n < nPoints; n++) {
                prop = (dataRight[n] / (2. * dataRecovered[n]));
                // needs checking! All the data in IAXO has the baseline include, so at this moment it can be
                // removed manually adding -260. In Cast, not.
                newSignalR->AddPoint(dataRight[n] - (Short_t)((prop) * (dataRecovered[n] / 2.)));
                // newSignalR->AddPoint(dataRight[n]-(Short_t)((prop)*(dataRecovered[n]/2. - 260))); //for
                // IAXO-D0
                newSignalL->AddPoint(dataLeft[n] - (Short_t)((1 - prop) * (dataRecovered[n] / 2.)));
                // newSignalL->AddPoint(dataLeft[n]-(Short_t)((1-prop)*(dataRecovered[n]/2. - 260))); //for
                // IAXO-D0
            }
            fOutputSignalEvent->AddSignal(*newSignalL);
            fOutputSignalEvent->AddSignal(*newSignalR);
        }

        if (leftSgnl == nullptr) {
            for (int n = 0; n < nPoints; n++) newSignalR->AddPoint(dataRight[n] - dataRecovered[n] / 2.);
            fOutputSignalEvent->AddSignal(*newSignalR);
        }

        if (rightSgnl == nullptr) {
            for (int n = 0; n < nPoints; n++) newSignalL->AddPoint(dataLeft[n] - dataRecovered[n] / 2.);
            fOutputSignalEvent->AddSignal(*newSignalL);
        }

        /*cout << "Channel recovered!! " << "proportion:" << endl;
        if( leftSgnl != nullptr && rightSgnl != nullptr )
            for( int n = 0; n < nPoints; n++ )
                cout << "Sample " << n << " : " << leftSgnl->GetData(n) << " + " << rightSgnl->GetData(n) << "
        = " << recoveredSignal->GetData(n) << endl; GetChar();*/

        delete recoveredSignal;
        delete newSignalL;
        delete newSignalR;
    }

    /*
  cout << "Channels after : " << fOutputSignalEvent->GetNumberOfSignals() << endl;
  GetChar();
    */

    return fOutputSignalEvent;
}

void TRestRawSignalRecoverChannelsProcess::GetAdjacentSignalIds(Int_t signalId, Int_t& idLeft, Int_t& idRight,
                                                                Int_t& idLeft2, Int_t& idRight2) {
#ifdef REST_DetectorLib
    for (int p = 0; p < fReadout->GetNumberOfReadoutPlanes(); p++) {
        TRestDetectorReadoutPlane* plane = fReadout->GetReadoutPlane(p);
        for (int m = 0; m < plane->GetNumberOfModules(); m++) {
            TRestDetectorReadoutModule* mod = plane->GetModule(m);
            // We iterate over all readout modules searching for the one that contains
            // our signal id
            if (mod->isDaqIDInside(signalId)) {
                // If we find it we use the readoutModule id, and the signalId
                // corresponding to the physical readout channel
                Int_t readoutChannelID = mod->DaqToReadoutChannel(signalId);

                idLeft = mod->GetChannel(readoutChannelID - 1)->GetDaqID();
                idRight = mod->GetChannel(readoutChannelID + 1)->GetDaqID();
                idLeft2 = mod->GetChannel(readoutChannelID - 2)->GetDaqID();
                idRight2 = mod->GetChannel(readoutChannelID + 2)->GetDaqID();

                return;
            }
        }
    }
#endif

    idLeft = -1;
    idRight = -1;
    idLeft2 = -1;
    idRight2 = -1;
}
