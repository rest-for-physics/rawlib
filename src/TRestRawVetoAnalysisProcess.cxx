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
/// TRestRawVetoAnalysisProcess ... needs to be documented
///
/// TO BE DOCUMENTED
///
/// <hr>
///
/// \warning **⚠ WARNING: REST is under continous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this code
/// up to date. Your feedback will be worth to support this software, please report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos, updating
/// information or adding/proposing new contributions. See also our [Contribution
/// Guide](https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md)
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2019-Nov:  First implementation
///             Cristina Margalejo/Javier Galan
///
/// 2020-Dec:  Added multi-VETO channel capability
///             Konrad Altenmuller
///
/// \class      TRestRawVetoAnalysisProcess
/// \author     Cristina Margalejo
/// \author     Javier Galan
/// \author     Konrad Altenmuller
///
/// <hr>
///
#include "TRestRawVetoAnalysisProcess.h"
using namespace std;

ClassImp(TRestRawVetoAnalysisProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawVetoAnalysisProcess::TRestRawVetoAnalysisProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// The path to the config file can be specified using full path,
/// absolute or relative.
///
/// If the file is not found then REST will try to find the file on
/// the default paths defined in REST Framework, usually at the
/// REST_PATH installation directory. Additional search paths may be
/// defined using the parameter `searchPath` in globals section. See
/// TRestMetadata description.
///
/// \param cfgFileName A const char* giving the path to an RML file.
///
TRestRawVetoAnalysisProcess::TRestRawVetoAnalysisProcess(char* cfgFileName) {
    Initialize();

    LoadConfig(cfgFileName);
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawVetoAnalysisProcess::~TRestRawVetoAnalysisProcess() { delete fOutputRawSignalEvent; }

///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
///
void TRestRawVetoAnalysisProcess::LoadDefaultConfig() {
    SetName(this->ClassName());
    SetTitle("Default config");
}

///////////////////////////////////////////////
/// \brief Function to load the configuration from an external configuration
/// file.
///
/// If no configuration path is defined in TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the
/// correspondig TRestRawVetoAnalysisProcess section inside the RML.
///
void TRestRawVetoAnalysisProcess::LoadConfig(std::string cfgFilename, std::string name) {
    if (LoadConfigFromFile(cfgFilename, name)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Function to use in initialization of process members before starting
/// to process the event
///
void TRestRawVetoAnalysisProcess::InitProcess() {
    // For example, try to initialize a pointer to existing metadata
    // accessible from TRestRun
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name and library version
///
void TRestRawVetoAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    // SetLibraryVersion(LIBRARY_VERSION);

    fInputRawSignalEvent = NULL;
    fOutputRawSignalEvent = new TRestRawSignalEvent();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawVetoAnalysisProcess::ProcessEvent(TRestEvent* evInput) {
    fInputRawSignalEvent = (TRestRawSignalEvent*)evInput;

    *fOutputRawSignalEvent = *fInputRawSignalEvent;

    /*
    cout << "I am in process " << GetProcessName() << endl;

    cout << "With event ID : " << fOutputRawSignalEvent->GetID() << endl;

    fOutputRawSignalEvent->PrintEvent();
    GetChar();
    */

    Int_t nVetoes = fVetoSignalId.size();  // how many veto channels are there?
    map<int, Double_t> VetoMaxPeakAmplitude_map;
    map<int, Double_t> VetoPeakTime_map;

    fOutputRawSignalEvent->SetBaseLineRange(fBaseLineRange);
    fOutputRawSignalEvent->SetRange(fRange);

    VetoMaxPeakAmplitude_map.clear();
    VetoPeakTime_map.clear();

    // cout << nVetoes << endl;

    // get veto IDs (work in progress...)
    // TiXmlElement *vetoDefinition = GetElement("veto");
    //
    // for (unsigned int i=0; i<fVetoSystem.size(); i++){
    //  	cout << vetoDefinition->size() << endl;
    // }

    // iterate over vetoes
    for (unsigned int i = 0; i < nVetoes; i++) {
        if (fOutputRawSignalEvent->GetSignalIndex(fVetoSignalId[i]) !=
            -1) {  // Checks if channel (fVetoSignalID) participated in the event. If not, it is -1
            // We extract the parameters from the veto signal
            TRestRawSignal* sgnl = fOutputRawSignalEvent->GetSignalById(fVetoSignalId[i]);

            // And at the end we remove the signal from the event
            fOutputRawSignalEvent->RemoveSignalWithId(fVetoSignalId[i]);

            // Save two maps with (veto panel ID, max amplitude) and (veto panel ID, peak time)
            VetoMaxPeakAmplitude_map[fVetoSignalId[i]] = sgnl->GetMaxPeakValue();
            VetoPeakTime_map[fVetoSignalId[i]] = sgnl->GetMaxPeakBin();
        }
    }

    SetObservableValue("PeakTime", VetoPeakTime_map);
    SetObservableValue("MaxPeakAmplitude", VetoMaxPeakAmplitude_map);

    /*
    SetObservableValue("PeakTime", VetoPeakTime);                  // Cristina
    SetObservableValue("MaxPeakAmplitude", VetoMaxPeakAmplitude);  // Cristina
    */

    /*
    cout << "++++++++++++++++++++++++++" << endl;
    cout << "Signal removed" << endl;
    fOutputRawSignalEvent->PrintEvent();
    cout << "Signal removed" << endl;
    cout << "++++++++++++++++++++++++++" << endl;
    GetChar();
    */

    if (GetVerboseLevel() >= REST_Debug) {
        fOutputRawSignalEvent->PrintEvent();

        if (GetVerboseLevel() >= REST_Extreme) GetChar();
    }

    return fOutputRawSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function reading input parameters from the RML
/// TRestRawVetoAnalysisProcess section
///
void TRestRawVetoAnalysisProcess::InitFromConfigFile() {
    fVetoSignalId = StringToElements(GetParameter("vetoSignalId", "-1"), ",");
    fBaseLineRange = StringTo2DVector(GetParameter("baseLineRange", "(5,55)"));
    fRange = StringTo2DVector(GetParameter("range", "(10,500)"));
}

///////////////////////////////////////////////
/// \brief It prints out the process parameters stored in the
/// metadata structure
///
void TRestRawVetoAnalysisProcess::PrintMetadata() {
    BeginPrintProcess();

    // Print output metadata using, metadata << endl;

    for (unsigned int i = 0; i < fVetoSignalId.size(); i++)
        metadata << "Veto signal id : " << fVetoSignalId[i] << endl;

    EndPrintProcess();
}
