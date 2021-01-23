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
/// TRestRawVetoAnalysisProcess allows to define several signal IDs as
/// vetoes and to group them. This process adds the observables veto_PeakTime
/// and veto_MaxPeakAmplitude to the analysis tree.
///
/// ### RML file structure
///
/// To define a veto you have two options:
///
/// Option 1: Define the veto by adding a parameter "vetoSignalId" with a comma-separated list of the veto
/// signal IDs.
/// In this case the observables "veto_PeakTime" and "veto_MaxPeakAmplitude" are added
/// to the analysis tree. Each observable contains a map with a different key for each signal ID.
/// \code
/// <addProcess type="TRestRawVetoAnalysisProcess" name="veto" value='ON' verboseLevel="info"
/// vetoSignalId="4622,4624,..." >
/// \endcode
/// You can add parameters "baseLineRange" and "range":
/// \code
/// <parameter name="baseLineRange" value="(10,100)" />
///
/// <parameter name="range" value="(10,500)" />
/// \endcode
///
/// Option 2: Put the vetoes in groups by adding xml blocks "vetoGroup" with the parameters "name" and
/// "signalIDs".
/// \code
/// <vetoGroup name="top" signalIDs="4624,4626" />
///
/// <vetoGroup name="front" signalIDs="4660" />
///
/// <vetoGroup name="left" signalIDs="1,2,3,4,5,6" />
/// \endcode
/// In this case, for each group a different pair of observables is saved.
/// In this example they would be named "veto_PeakTime_top", "veto_PeakTime_front","veto_PeakTime_left"
/// (and the same for "MaxPeakAmplitude"), where each again contains a map with the signal ID as key.
///
/// ### Methods to retrieve metadata
///
/// The method GetVetoSignalIDs() returns a vector<double> of the veto signal IDs, if the vetoes were defined
/// using option 1.
/// In case the vetoes were defined in groups, one can use the method GetVetoGroups(),
/// which returns a std::pair<vector<string>,vector<string>>, which contains in the first entry the name of
/// the veto group,
/// and in the second the comma separated string of the corresponding signal IDs. The signal IDs can
/// susequently be converted
/// into a vector<double> by using the TRestStringHelper::StringToElements() method.
///
/// <hr>
///
/// \warning **⚠ WARNING: REST is under continous development.** This documentation
/// is offered to you by the REST community. Your HELP is needed to keep this code
/// up to date. Your feedback will be worth to support this software, please report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
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
/// 2019-Nov:  First implementation
///             Cristina Margalejo/Javier Galan
///
/// 2020-Dec:  Added multi-VETO channel capability
///             Konrad Altenmueller
///
/// 2021-Jan:  Added veto groups and observables accordingly
///		Konrad Altenmueller
///
/// \class      TRestRawVetoAnalysisProcess
/// \author     Cristina Margalejo
/// \author     Javier Galan
/// \author     Konrad Altenmueller
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
    SetLibraryVersion(LIBRARY_VERSION);

    fInputRawSignalEvent = NULL;
    fOutputRawSignalEvent = new TRestRawSignalEvent();
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawVetoAnalysisProcess::ProcessEvent(TRestEvent* evInput) {
    fInputRawSignalEvent = (TRestRawSignalEvent*)evInput;

    *fOutputRawSignalEvent = *fInputRawSignalEvent;

    map<int, Double_t> VetoMaxPeakAmplitude_map;
    map<int, Double_t> VetoPeakTime_map;

    fOutputRawSignalEvent->SetBaseLineRange(fBaseLineRange);
    fOutputRawSignalEvent->SetRange(fRange);

    VetoMaxPeakAmplitude_map.clear();
    VetoPeakTime_map.clear();

    // ***** debugging *****
    /* cout << "******************" << endl;
    // cout << "I am in process " << GetProcessName() << endl;
    cout << "event ID : " << fOutputRawSignalEvent->GetID() << endl;
    cout << "number of signals: " << fOutputRawSignalEvent->GetNumberOfSignals()
    << endl;
    cout << "signal IDs : ";
    fOutputRawSignalEvent->PrintSignalIds();
    cout  << endl;
    for (unsigned int i=0; i< fOutputRawSignalEvent->GetNumberOfSignals(); i++){
        TRestRawSignal* debug = fOutputRawSignalEvent->GetSignal(i);
        cout << "signal ID: " << debug->GetSignalID() << " Amp: " <<
    debug->GetMaxPeakValue() << endl;
    }
    */
    // *** end debugging ***

    // **************************************************************
    // if list of veto Ids without groups is given ******************
    // **************************************************************

    if (fVetoSignalId[0] != -1) {
        // iterate over vetoes
        for (unsigned int i = 0; i < fVetoSignalId.size(); i++) {
            // cout << "ID: "<< fVetoSignalId[i] << " Index: " <<
            // fOutputRawSignalEvent->GetSignalIndex(fVetoSignalId[i]) << "; ";

            // Checks if channel (fVetoSignalId) participated in the event. If not, it
            // is -1
            if (fOutputRawSignalEvent->GetSignalIndex(fVetoSignalId[i]) != -1) {
                // We extract the parameters from the veto signal
                TRestRawSignal* sgnl = fOutputRawSignalEvent->GetSignalById(fVetoSignalId[i]);
                // cout << "ID: " << fVetoSignalId[i] << " Amp: " <<
                // sgnl->GetMaxPeakValue() << endl;

                // Save two maps with (veto panel ID, max amplitude) and (veto panel ID,
                // peak time)
                VetoMaxPeakAmplitude_map[fVetoSignalId[i]] = sgnl->GetMaxPeakValue();
                VetoPeakTime_map[fVetoSignalId[i]] = sgnl->GetMaxPeakBin();
                // We remove the signal from the event
                fOutputRawSignalEvent->RemoveSignalWithId(fVetoSignalId[i]);

                // cout << "ID: " << fVetoSignalId[i] << " Amp: " <<
                // sgnl->GetMaxPeakValue() << endl;
                // cout << "********" << endl;
            }
        }

        // ***** debugging *****
        /*
        cout << endl;
        cout << "Observables Added: " << endl;
        cout << "Map size: " << VetoMaxPeakAmplitude_map.size() << endl;
        for (map<int, double>::const_iterator it = VetoMaxPeakAmplitude_map.begin();
        it != VetoMaxPeakAmplitude_map.end(); ++it){
            cout << "ID: " << it->first << " Amplitude: " << it->second;
            }
        cout << endl;
        */
        // *** end debugging ***

        SetObservableValue("PeakTime", VetoPeakTime_map);
        SetObservableValue("MaxPeakAmplitude", VetoMaxPeakAmplitude_map);
    }

    // ***************************************************************
    // if the veto ids are defined within the veto groups ************
    // ***************************************************************

    // create observable names for veto groups
    for (unsigned int i = 0; i < fVetoGroupNames.size(); i++) {
        fPeakTime.push_back("PeakTime_" + fVetoGroupNames[i]);
        fPeakAmp.push_back("MaxPeakAmplitude_" + fVetoGroupNames[i]);
    }

    if (fVetoSignalId[0] == -1) {
        // iterate over veto groups
        for (unsigned int i = 0; i < fVetoGroupNames.size(); i++) {
            // iterate over vetoes in each group
            vector<double> groupIds = StringToElements(fVetoGroupIds[i], ",");
            for (unsigned int j = 0; j < groupIds.size(); j++) {
                // Checks if channel (fVetoSignalId) participated in the event. If not,
                // it is -1
                if (fOutputRawSignalEvent->GetSignalIndex(groupIds[j]) != -1) {
                    // We extract the parameters from the veto signal
                    TRestRawSignal* sgnl = fOutputRawSignalEvent->GetSignalById(groupIds[j]);
                    // Save two maps with (veto panel ID, max amplitude) and (veto panel
                    // ID, peak time)
                    VetoMaxPeakAmplitude_map[groupIds[j]] = sgnl->GetMaxPeakValue();
                    VetoPeakTime_map[groupIds[j]] = sgnl->GetMaxPeakBin();
                    // We remove the signal from the event
                    fOutputRawSignalEvent->RemoveSignalWithId(groupIds[j]);
                }
            }
            SetObservableValue(fPeakTime[i], VetoPeakTime_map);
            SetObservableValue(fPeakAmp[i], VetoMaxPeakAmplitude_map);

            VetoMaxPeakAmplitude_map.clear();
            VetoPeakTime_map.clear();
        }
    }

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
    fBaseLineRange = StringTo2DVector(GetParameter("baseLineRange", "(5,55)"));
    fRange = StringTo2DVector(GetParameter("range", "(10,500)"));

    // **************************************************************
    // ***** Vetoes are defined as a single list ********************
    // **************************************************************

    fVetoSignalId = StringToElements(GetParameter("vetoSignalId", "-1"), ",");

    // **************************************************************
    // ***** Vetoes are defined in groups ***************************
    // **************************************************************

    // Read all the info from the veto group definitions

    TiXmlElement* vetoDefinition = GetElement("vetoGroup");

    while (vetoDefinition != NULL) {
        fVetoGroupNames.push_back(GetFieldValue("name", vetoDefinition));
        fVetoGroupIds.push_back(GetFieldValue("signalIDs", vetoDefinition));
        vetoDefinition = GetNextElement(vetoDefinition);
    }

    // Stop, in case signalIDs and groups are defined separately
    if (fVetoSignalId[0] != -1 && fVetoGroupNames.size() > 0) {
        cout << "Error: veto groups and veto IDs defined separately!" << endl;
        GetChar();
    }
}
///////////////////////////////////////////////
/// \brief It prints out the process parameters stored in the
/// metadata structure
///
void TRestRawVetoAnalysisProcess::PrintMetadata() {
    BeginPrintProcess();

    // Print output metadata using, metadata << endl;
    for (unsigned int i = 0; i < fVetoGroupNames.size(); i++) {
        metadata << "Veto group " << fVetoGroupNames[i] << " signal IDs: " << fVetoGroupIds[i] << endl;
    }

    if (fVetoSignalId[0] != -1) {
        for (unsigned int i = 0; i < fVetoSignalId.size(); i++) {
            metadata << "Veto signal ID: " << fVetoSignalId[i] << endl;
        }
    } else {
        metadata << " " << endl;
        metadata << "All veto signal IDs: ";
        for (unsigned int i = 0; i < fVetoGroupIds.size() - 1; i++) {
            metadata << fVetoGroupIds[i] << ",";
        }
        metadata << fVetoGroupIds[fVetoGroupIds.size() - 1] << endl;
    }

    EndPrintProcess();
}
