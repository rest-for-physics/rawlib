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
/// The TRestRawSignalShapingProcess allows to convolute a given time signal
/// response with the TRestRawSignal input signals found inside the input
/// TRestRawSignalEvent. This process may serve for conditioning the signals
/// produced in a MonteCarlo simulation and emulate those from real
/// electronics.
///
/// For the moment, only gaussian response has been implemented.
///
/// The different parameters allowed in this process are:
///
/// * **shapingType**: It defines the type of convolution to be performed.
///     - gaus : It produces a gausian convolution.
///     - shaper : It produces a shaping following traditional shaper
///               waveforms.
///     - shaperSin : It produces a shaping following traditional shaper
///               waveforms, it includes a sinusoidal effect.
///     - responseFile : Uses a file providing a custom response (TODO).
///
/// * **shapingTime** : The standard deviation of the gaussian convolution,
///                     or the shaping time on shaper models. Defined in
///                     samples unit.
/// * **gain** : A factor to amplify or attenuate the signal.
/// * **responseFile** : A response file to be used in case the shapingType
/// is defined to use a response file.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
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
/// 2016-March: First implementation
///             Xinglong
///
/// 2018-March: Transfered to TRestRawSignal
///             Javier Galan
///
/// \class      TRestRawSignalShapingProcess
/// \author     Xinglong
/// \author     Javier Galan
///
/// <hr>
///
#include "TRestRawSignalShapingProcess.h"

using namespace std;

#include <TFile.h>
#include <TMath.h>

ClassImp(TRestRawSignalShapingProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignalShapingProcess::TRestRawSignalShapingProcess() { Initialize(); }

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
TRestRawSignalShapingProcess::TRestRawSignalShapingProcess(const char* configFilename) {
    Initialize();
    LoadConfigFromFile(configFilename);
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignalShapingProcess::~TRestRawSignalShapingProcess() { delete fOutputSignalEvent; }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestRawSignalShapingProcess::Initialize() {
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
void TRestRawSignalShapingProcess::LoadConfig(const string& configFilename, const string& name) {
    LoadConfigFromFile(configFilename, name);
}

///////////////////////////////////////////////
/// \brief Process initialization. Observable names are interpreted and auxiliar
/// observable members, related to VolumeEdep, MeanPos, TracksCounter, TrackEDep
/// observables defined in TRestGeant4AnalysisProcess are filled at this stage.
///
void TRestRawSignalShapingProcess::InitProcess() {
    /*
     * NOT IMPLEMENTED. TODO To use a generic response from a
     * predefined TRestDetectorSignal
     *
     * For the moment we do only a gaussian shaping"
     * /

    responseSignal = new TRestRawSignal();

    if( fShapingType == "responseFile" )
    {
        TString fullPathName = (TString) getenv("REST_PATH") + "/data/signal/" +
    fResponseFilename; TFile *f = new TFile(fullPathName); responseSignal =
    (TRestRawSignal *) f->Get("signal Response"); f->Close();
    }

    if( GetVerboseLevel() >= REST_Debug )
    {
        CreateCanvas();
        fCanvas->Draw();
    }

    if( fShapingType == "gaus" )
    {
        responseSignal->InitGaussian( 100, 100, 30, 200 );

        if( GetVerboseLevel() >= REST_Debug )
        {
            responseSignal->GetGraph()->Draw();
            fCanvas->Update();
            GetChar();
        }
    }
    */
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestRawSignalShapingProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputSignalEvent = (TRestRawSignalEvent*)inputEvent;

    if (fInputSignalEvent->GetNumberOfSignals() <= 0) {
        return nullptr;
    }

    std::vector<double> response;
    Int_t Nr = 0;

    /// This is done for every event however we could do it inside InitProcess!
    /// It is the response function. Does not change from event ot event
    if (fShapingType == "gaus") {
        Int_t cBin = (Int_t)(fShapingTime * 3.5);
        Nr = 2 * cBin;
        Double_t sigma = fShapingTime;

        response.resize(Nr);

        for (int i = 0; i < Nr; i++) {
            response[i] = TMath::Exp(-0.5 * (i - cBin) * (i - cBin) / sigma / sigma);
            response[i] = response[i] / TMath::Sqrt(2 * M_PI) / sigma;
        }
    } else if (fShapingType == "exponential") {
        Nr = (Int_t)(5 * fShapingTime);

        response.resize(Nr);

        for (int i = 0; i < Nr; i++) {
            Double_t coeff = ((Double_t)i) / fShapingTime;
            response[i] = TMath::Exp(-coeff);
        }
    } else if (fShapingType == "shaper") {
        Nr = (Int_t)(5 * fShapingTime);

        response.resize(Nr);

        for (int i = 0; i < Nr; i++) {
            Double_t coeff = ((Double_t)i) / fShapingTime;
            response[i] = TMath::Exp(-3. * coeff) * coeff * coeff * coeff;
        }
    } else if (fShapingType == "shaperSin") {
        Nr = (Int_t)(5 * fShapingTime);

        response.resize(Nr);

        for (int i = 0; i < Nr; i++) {
            Double_t coeff = ((Double_t)i) / fShapingTime;
            response[i] = TMath::Exp(-3. * coeff) * coeff * coeff * coeff * sin(coeff);
        }
    } else {
        if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Warning)
            cout << "REST WARNING. Shaping type : " << fShapingType << " is not defined!!" << endl;
        return nullptr;
    }

    // Making sure that response integral is 1, and applying the gain
    Double_t sum = 0;
    for (int n = 0; n < Nr; n++) sum += response[n];
    for (int n = 0; n < Nr; n++) response[n] = response[n] * fShapingGain / sum;

    for (int n = 0; n < fInputSignalEvent->GetNumberOfSignals(); n++) {
        TRestRawSignal shapingSignal = TRestRawSignal();
        TRestRawSignal inSignal = *fInputSignalEvent->GetSignal(n);
        Int_t nBins = inSignal.GetNumberOfPoints();

        vector<double> out(nBins);
        for (int m = 0; m < nBins; m++) out[m] = 0;

        for (int m = 0; m < nBins; m++) {
            if (inSignal.GetData(m) >= 0) {
                if (fShapingType == "gaus") {
                    for (int n = -Nr / 2; m + n < nBins && n < Nr / 2; n++)
                        if (m + n >= 0) out[m + n] += response[n + Nr / 2] * inSignal.GetData(m);
                } else
                    for (int n = 0; m + n < nBins && n < Nr; n++)
                        out[m + n] += response[n] * inSignal.GetData(m);
            }
        }

        for (int i = 0; i < nBins; i++) {
            shapingSignal.AddPoint((Short_t)round(out[i]));
        }
        shapingSignal.SetSignalID(inSignal.GetSignalID());

        fOutputSignalEvent->AddSignal(shapingSignal);
    }

    return fOutputSignalEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestRawSignalShapingProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}
