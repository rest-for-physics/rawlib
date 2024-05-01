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
/// The TRestRawSignalRemoveChannelsProcess allows to remove a selection
/// daq channels ids from a TRestRawSignalEvent. The channels should be
/// provided through the corresponding section at the RML configuration file.
///
/// An application of this process is to evaluate the impact on the detector
/// response due to the absence of signal on some channels, or dead channels.
///
/// The following example will remove the channels with signal ids 17,19,27
/// and all the signals between 67 and 76 from the TRestRawSignalEvent output.
///
/// \code
/// <TRestRawSignalRemoveChannelsProcess name="rmChannels" title="Removing few
/// channels" verboseLevel="debug" >
///     <removeChannel id="17" />
///     <removeChannel id="19" />
///     <removeChannel id="27" />
///     <removeChannels range="(67,76)" />
/// </TRestRawSignalRemoveChannelsProcess>
/// \endcode
///
/// <hr>
///
/// \warning ** REST is under continous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code up to date. Your feedback will be worth to support this software, please
/// report any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos, updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2017-November: First implementation of TRestRawSignalRemoveChannelsProcess.
///             Javier Galan
///
/// \class      TRestRawSignalRemoveChannelsProcess
/// \author     Javier Galan
///
/// <hr>
///
#include "TRestRawSignalRemoveChannelsProcess.h"

#include <TRestRawReadoutMetadata.h>

using namespace std;

ClassImp(TRestRawSignalRemoveChannelsProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalRemoveChannelsProcess::TRestRawSignalRemoveChannelsProcess() { Initialize(); }

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
TRestRawSignalRemoveChannelsProcess::TRestRawSignalRemoveChannelsProcess(const char* configFilename) {
    Initialize();

    if (LoadConfigFromFile(configFilename) == -1) LoadDefaultConfig();

    PrintMetadata();
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalRemoveChannelsProcess::~TRestRawSignalRemoveChannelsProcess() { delete fOutputSignalEvent; }

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawSignalRemoveChannelsProcess::LoadDefaultConfig() {
    SetName("removeChannels-Default");
    SetTitle("Default config");
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalRemoveChannelsProcess::Initialize() {
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
/// corresponding TRestGeant4AnalysisProcess section inside the RML.
///
void TRestRawSignalRemoveChannelsProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name) == -1) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalRemoveChannelsProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputSignalEvent = (TRestRawSignalEvent*)inputEvent;
    fInputSignalEvent->InitializeReferences(GetRunInfo());

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputSignalEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr) {
        cerr << "TRestRawBaseLineCorrectionProcess::ProcessEvent: readout metadata is null" << endl;
        exit(1);
    }

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal* signal = fInputSignalEvent->GetSignal(n);

        bool removeSignal = false;

        // Check if the channel ID matches any specified for removal
        for (unsigned int x = 0; x < fChannelIds.size() && !removeSignal; x++) {
            if (signal->GetID() == fChannelIds[x]) {
                removeSignal = true;
                break;
            }
        }

        // Check if the channel type matches any specified for removal
        if (!removeSignal) {
            string channelType =
                fInputSignalEvent->GetReadoutMetadata()->GetTypeForChannelDaqId(signal->GetSignalID());
            if (find(fChannelTypes.begin(), fChannelTypes.end(), channelType) != fChannelTypes.end()) {
                removeSignal = true;
            }
        }

        if (!removeSignal) {
            fOutputSignalEvent->AddSignal(*signal);
        }

        // Logging messages
        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
            cout << "Channel ID : " << signal->GetID() << endl;
        }

        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug && removeSignal) {
            cout << "Removing channel id : " << signal->GetID() << endl;
        }
    }

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Extreme) {
        GetChar();
    }

    return fOutputSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function reading input parameters from the RML
/// TRestDetectorSignalToRawSignalProcess metadata section
///
void TRestRawSignalRemoveChannelsProcess::InitFromConfigFile() {
    size_t pos = 0;

    string removeChannelDefinition;
    while ((removeChannelDefinition = GetKEYDefinition("removeChannel", pos)) != "") {
        Int_t id = StringToInteger(GetFieldValue("id", removeChannelDefinition));
        fChannelIds.push_back(id);
    }

    pos = 0;
    while ((removeChannelDefinition = GetKEYDefinition("removeChannels", pos)) != "") {
        TVector2 v = StringTo2DVector(GetFieldValue("range", removeChannelDefinition));
        if (v.X() >= 0 && v.Y() >= 0 && v.Y() > v.X())
            for (int n = (Int_t)v.X(); n <= (Int_t)v.Y(); n++) {
                fChannelIds.push_back(n);
            }
    }

    pos = 0;
    while ((removeChannelDefinition = GetKEYDefinition("removeChannels", pos)) != "") {
        string type = GetFieldValue("type", removeChannelDefinition);
        fChannelTypes.push_back(type);
    }
}
