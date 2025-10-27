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
/// TRestRawSignalAddNoiseProcess is a process that allows to add a
/// random gaussian noise to the input TRestRawSignalEvent.
///
/// The process makes use of TRestRawSignal::GetWhiteNoiseSignal which simply
/// adds a random gaussian value centered on zero and with a sigma given
/// by the parameter `noiseLevel`.
///
/// The process can be defined as follows:
///
/// \code
/// <TRestRawSignalAddNoiseProcess name="noise" >
///     <parameter name="noiseLevel" value="10" />
/// </TRestRawSignalAddNoiseProcess>
/// \endcode
///
/// or
///
/// \code
/// <TRestRawSignalAddNoiseProcess name="noise" noiseLevel="10" />
/// \endcode
///
/// <hr>
///
/// \warning **⚠ REST is under continuous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code up to date. Your feedback will be worth to support this software, please
/// report any problems/suggestions you may find will using it at [The REST Framework
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
/// 2016-February: First concept and implementation of TRestRawSignalAddNoise
/// process.
/// \author     Javier Gracia
///
/// \class TRestRawSignalAddNoiseProcess
///
/// <hr>
///

#include "TRestRawSignalAddNoiseProcess.h"

#include <TFile.h>

using namespace std;

ClassImp(TRestRawSignalAddNoiseProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalAddNoiseProcess::TRestRawSignalAddNoiseProcess() { Initialize(); }

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
TRestRawSignalAddNoiseProcess::TRestRawSignalAddNoiseProcess(const char* configFilename) {
    Initialize();
    if (LoadConfigFromFile(configFilename) == -1) {
        LoadDefaultConfig();
    }
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalAddNoiseProcess::~TRestRawSignalAddNoiseProcess() {
    delete fOutputSignalEvent;
    // TRestRawSignalAddNoiseProcess destructor
}

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawSignalAddNoiseProcess::LoadDefaultConfig() {
    SetName("addSignalNoiseProcess-Default");
    SetTitle("Default config");
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalAddNoiseProcess::Initialize() {
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
void TRestRawSignalAddNoiseProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name) == -1) {
        LoadDefaultConfig();
    }
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalAddNoiseProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputSignalEvent = (TRestRawSignalEvent*)inputEvent;

    if (fInputSignalEvent->GetNumberOfSignals() <= 0) {
        return nullptr;
    }

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal noiseSignal;

        // Assign ID and add noise
        fInputSignalEvent->GetSignal(n)->GetWhiteNoiseSignal(&noiseSignal, fNoiseLevel);
        noiseSignal.SetSignalID(fInputSignalEvent->GetSignal(n)->GetSignalID());

        fOutputSignalEvent->AddSignal(noiseSignal);
    }

    return fOutputSignalEvent;
}
