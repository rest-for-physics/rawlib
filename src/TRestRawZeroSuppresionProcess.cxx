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
/// The TRestRawZeroSuppresionProcess identifies the points that are over
/// threshold from the input TRestRawSignalEvent. The resulting points remains as a 
/// TRestRawSignalEvent. This process mimic the zero suppression done in the electronics
/// and also substract the baseline.
/// The different parameters required by this process are:
/// * baselineRange : A 2D-vector definning the range, in number of bins,
/// where the baseline properties will be calculated.
/// * integralRange : A 2D-vector definning the time window, in number of bins,
/// where the signal will be considered.
/// * pointThreshold : The number of sigmas over the baseline flunctuations to
/// consider a point is over the threshold.
/// * pointsOverThreshold : The number of consecutive points over threshold
/// required to consider them as a physical signal.
/// * signalThreshold : The number of sigmas a set of consecutive points
/// identified over threshold must be over the baseline fluctuations to be
/// finally considered a physical signal.
/// * pointsFlatThreshold : A parameter to help removing the un-physical or
/// un-expected tail that follows the physical signal. \TODO more details?
///
/// \TODO Add description of observables here.
///
///
/// An example of definition of this process inside a data processing chain,
/// inside the `<TRestProcessRunner>` section.
///
/// The values given between `${}` are enviroment variables that can be defined
/// at the system or at the `<globals>` section. See also TRestMetadata for
/// additional details.
///
/// \code
///   <addProcess type="TRestRawZeroSuppresionProcess" name="zS" value="ON"
///               baseLineRange="(${BL_MIN},${BL_MAX})"
///               integralRange="(${INT_MIN},${INT_MAX})"
///               pointThreshold="${POINT_TH}"
///               pointsOverThreshold="${NPOINTS}"
///               signalThreshold="${SGNL_TH}"
///               observable="all"
///               verboseLevel="silent"   />
/// \endcode
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2016-January: Conception and implementation of signal zero suppression
/// process.
///               Javier Galan
///
/// \class      TRestRawZeroSuppresionProcess
/// \author     Javier Galan
/// \author     Kaixiang Ni
///
/// 2022 January Removing conversion to TRestDetectorSignal and moving to raw library, 
/// now it returns a TRestRawSignalEvent after appliying zero suppression
/// 
/// ! author    JuanAn Garcia
///
/// <hr>
///

#include "TRestRawZeroSuppresionProcess.h"
#include <numeric>
using namespace std;

ClassImp(TRestRawZeroSuppresionProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawZeroSuppresionProcess::TRestRawZeroSuppresionProcess() { Initialize(); }

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
TRestRawZeroSuppresionProcess::TRestRawZeroSuppresionProcess(char* cfgFileName) {
    Initialize();

    LoadConfig(cfgFileName);
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawZeroSuppresionProcess::~TRestRawZeroSuppresionProcess() { delete fOutputSignalEvent; }

///////////////////////////////////////////////
/// \brief Method to load the default config in absence of RML input
///
void TRestRawZeroSuppresionProcess::LoadDefaultConfig() {
    SetName("signalZeroSuppresionProcess-Default");
    SetTitle("Default config");

    cout << "Signal to hits metadata not found. Loading default values" << endl;

    TVector2 fBaseLineRange = TVector2(10, 90);
    fPointThreshold = 2.;
    fSignalThreshold = 2.;
    fNPointsOverThreshold = 10;
    fBaseLineCorrection = false;
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
/// correspondig TRestGeant4AnalysisProcess section inside the RML.
///
void TRestRawZeroSuppresionProcess::LoadConfig(std::string cfgFilename, std::string name) {
    if (LoadConfigFromFile(cfgFilename, name)) LoadDefaultConfig();
}

///////////////////////////////////////////////
/// \brief Input/output event types declaration and section name.
///
void TRestRawZeroSuppresionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fRawSignalEvent = NULL;
    fOutputSignalEvent = new TRestRawSignalEvent();
}

///////////////////////////////////////////////
/// \brief Process initialization. It identifies if baseline has been
/// already zero-ed by a previous process.
///
/// Personal note (TODO): Should be the baseline zero-ed by
/// TRestRawSignalAnalysisProcess? It is that necessary?
///
void TRestRawZeroSuppresionProcess::InitProcess() {
    // !!!!!!!!!!!! BASELINE CORRECTION !!!!!!!!!!!!!!
    // TRestRawSignalAnalysisProcess subtracts baseline. Baseline is double value,
    // but data points in TRestRawSignalAnalysisProcess is only short integer
    // type. So we need to correct this by adding decimal part back.

    // 2019-Nov-01: Baseline correction in TRestRawSignalAnalysisProcess has been removed. Retrieving
    // parameters from other processes as it is done here is ok. However, I would recommend
    // to define a globals.xml file where we define parameters that are then used by any process inside the
    // RML itself.
    /*
    fBaseLineCorrection = false;
    for (int i = 0; i < fFriendlyProcesses.size(); i++) {
        if ((string)fFriendlyProcesses[i]->ClassName() == "TRestRawSignalAnalysisProcess") {
            fBaseLineCorrection = true;

            // setting parameters to the same as TRestRawSignalAnalysisProcess
            fBaseLineRange = StringTo2DVector(fFriendlyProcesses[i]->GetParameter("baseLineRange", "(5,55)"));
            fIntegralRange =
                StringTo2DVector(fFriendlyProcesses[i]->GetParameter("integralRange", "(10,500)"));
            fPointThreshold = StringToDouble(fFriendlyProcesses[i]->GetParameter("pointThreshold", "2"));
            fNPointsOverThreshold =
                StringToInteger(fFriendlyProcesses[i]->GetParameter("pointsOverThreshold", "5"));
            fSignalThreshold = StringToDouble(fFriendlyProcesses[i]->GetParameter("signalThreshold", "5"));
        }
    }
    */
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawZeroSuppresionProcess::ProcessEvent(TRestEvent* evInput) {
    fRawSignalEvent = (TRestRawSignalEvent*)evInput;

    fRawSignalEvent->SetBaseLineRange(fBaseLineRange);
    fRawSignalEvent->SetRange(fIntegralRange);

    Int_t numberOfSignals = fRawSignalEvent->GetNumberOfSignals();

    Double_t totalIntegral = 0;
    Double_t rejectedSignal = 0;
    int totoalflatN = 0;
    map<int, int> flattailmap;  // signal id --> number of points in abnormal flat tail
    for (int n = 0; n < numberOfSignals; n++) {
        TRestRawSignal* s = fRawSignalEvent->GetSignal(n);

        int Nbefore;
        if (fNPointsFlatThreshold != 512) {
            s->InitializePointsOverThreshold(TVector2(fPointThreshold, fSignalThreshold),
                                             fNPointsOverThreshold, 512);
            Nbefore = s->GetPointsOverThreshold().size();
        }
        s->InitializePointsOverThreshold(TVector2(fPointThreshold, fSignalThreshold), fNPointsOverThreshold, fNPointsFlatThreshold);

        int Nafter = s->GetPointsOverThreshold().size();
        // cout << fRawSignalEvent->GetID() << " " << s->GetID() << " " << Nbefore << " " << Nafter << endl;
        if (Nafter != Nbefore) {
            flattailmap[s->GetID()] = Nbefore - Nafter;
            totoalflatN += Nbefore - Nafter;
        }

        TRestRawSignal sgn(s->GetNumberOfPoints());
        sgn.SetID(s->GetID());

        std::vector<Int_t> pOver = s->GetPointsOverThreshold();
        for (int n = 0; n < pOver.size(); n++) {
            int j = pOver[n];
            sgn.IncreaseBinBy(j, s->GetData(j));
        }

        if (sgn.GetNumberOfPoints() > 0) {
            fOutputSignalEvent->AddSignal(sgn);
            totalIntegral += sgn.GetIntegral();
        } else {
            rejectedSignal++;
        }

        if (this->GetVerboseLevel() >= REST_Extreme) {
            fOutputSignalEvent->PrintEvent();
        }
    }

    SetObservableValue("flattail_map", flattailmap);
    SetObservableValue("FlatTails", totoalflatN);
    SetObservableValue("NFlatTailSignals", (int)flattailmap.size());
    SetObservableValue("NSignalsRejected", rejectedSignal);

    debug << "TRestRawZeroSuppresionProcess. Signals added : " << fOutputSignalEvent->GetNumberOfSignals() << endl;
    debug << "TRestRawZeroSuppresionProcess. Signals rejected : " << rejectedSignal << endl;
    debug << "TRestRawZeroSuppresionProcess. Threshold integral : " << totalIntegral << endl;

    if (fOutputSignalEvent->GetNumberOfSignals() <= 0) return nullptr;

    return fOutputSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestRawZeroSuppresionProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

///////////////////////////////////////////////
/// \brief Function to read input parameters from the RML
/// TRestRawZeroSuppresionProcess metadata section
///
void TRestRawZeroSuppresionProcess::InitFromConfigFile() {
    // keep up with TRestRawSignalAnalysisProcess
    fBaseLineRange = StringTo2DVector(GetParameter("baseLineRange", "(5,55)"));
    fIntegralRange = StringTo2DVector(GetParameter("integralRange", "(10,500)"));
    fPointThreshold = StringToDouble(GetParameter("pointThreshold", "2"));
    fNPointsOverThreshold = StringToInteger(GetParameter("pointsOverThreshold", "5"));
    fSignalThreshold = StringToDouble(GetParameter("signalThreshold", "5"));

    // introduced to prevent daq abnormal response: flat high signal tail
    fNPointsFlatThreshold = StringToInteger(GetParameter("pointsFlatThreshold", "512"));

}
