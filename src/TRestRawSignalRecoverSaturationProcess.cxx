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

/////////////////////////////////////////////////////////////////////////
/// Write the process description Here                                   
/// 
/// ### Parameters
/// Describe any parameters this process receives: 
/// * **parameter1**: This parameter ...
/// * **parameter2**: This parameter is ...
/// 
/// 
/// ### Examples
/// Give examples of usage and RML descriptions that can be tested.      
/// \code
///     <WRITE A CODE EXAMPLE HERE>
/// \endcode
/// 
/// ### Running pipeline example
/// Add the examples to a pipeline to guarantee the code will be running 
/// on future framework upgrades.                                        
/// 
/// 
/// Please, add any figure that may help to ilustrate the process        
/// 
/// \htmlonly <style>div.image img[src="trigger.png"]{width:500px;}</style> \endhtmlonly
/// ![An ilustration of the trigger definition](trigger.png)             
/// 
/// The png image should be uploaded to the ./images/ directory          
///                                                                      
///----------------------------------------------------------------------
///                                                                      
/// REST-for-Physics - Software for Rare Event Searches Toolkit 		    
///                                                                      
/// History of developments:                                             
///                                                                      
/// YEAR-Month: First implementation of TRestRawSignalRecoverSaturationProcess
/// WRITE YOUR FULL NAME 
///                                                                      
/// \class TRestRawSignalRecoverSaturationProcess                                               
/// \author: TODO. Write full name and e-mail:        aezquerro
///                                                                      
/// <hr>                                                                 
///                                                                      

#include "TRestRawSignalRecoverSaturationProcess.h"

ClassImp(TRestRawSignalRecoverSaturationProcess);

///////////////////////////////////////////////                          
/// \brief Default constructor                                          
///                                                                      
TRestRawSignalRecoverSaturationProcess::TRestRawSignalRecoverSaturationProcess() {
    Initialize();
}

///////////////////////////////////////////////                          
/// \brief Default destructor                                           
///                                                                      
TRestRawSignalRecoverSaturationProcess::~TRestRawSignalRecoverSaturationProcess() {
}

///////////////////////////////////////////////                          
/// \brief Function to initialize input/output event members and define  
/// the section name                                                     
///                                                                      
void TRestRawSignalRecoverSaturationProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);
    fAnaEvent = NULL;

    // Initialize here the values of class data members if needed       

}

///////////////////////////////////////////////                           
/// \brief Process initialization. Observable names can be re-interpreted 
/// here. Any action in the process required before starting event process 
/// might be added here.                                                 
///                                                                      
void TRestRawSignalRecoverSaturationProcess::InitProcess() {
    // Write here the jobs to do before processing
    // i.e., initialize histograms and auxiliary vectors,
    // read TRestRun metadata, or load additional rml sections

}

///////////////////////////////////////////////                          
/// \brief The main processing event function                           
///                                                                      
TRestEvent* TRestRawSignalRecoverSaturationProcess::ProcessEvent(TRestEvent * evInput) {
    fAnaEvent = (TRestRawSignalEvent*)evInput;

    // If cut condition matches the event will be not registered.
    if (ApplyCut()) return nullptr;

    // Write here the main logic of process: TRestRawSignalRecoverSaturationProcess
    // Read data from input event, write data to output event, and save observables to tree

    RESTDebug << "TRestRawSignalRecoverSaturationProcess::ProcessEvent. Event ID : " << fAnaEvent->GetID()
              << RESTendl;
    Double_t addedAmplitude = 0;
    Double_t addedIntegral = 0;
    Int_t nSignalsSaturated = 0;
    Int_t nSignalsRecovered = 0;
    for (int s = 0; s < fAnaEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* signal = fAnaEvent->GetSignal(s);

        if (!signal->IsADCSaturation(3))
            continue;
        nSignalsSaturated++;
        RESTDebug << "Processing signal " << s << RESTendl;

        Int_t maxPeakBin = signal->GetMaxPeakBin();
        Short_t maxValue = (*signal)[maxPeakBin];
        std::vector<size_t> saturatedBins;

        for (size_t i = (size_t)maxPeakBin; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if ((*signal)[i] == maxValue) saturatedBins.push_back(i);
            else break; // Stop when the signal stops being saturated
        }

        RESTDebug << "Saturated bins: ";
        for (auto bin : saturatedBins) {
            RESTDebug << bin << " ";
        }
        RESTDebug << RESTendl;
        RESTDebug << "with value " << maxValue << RESTendl;

        // Create TGraph with the not saturated bins for the fit
        TGraph* g = new TGraph();
        for (size_t i = 0; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if (std::find(saturatedBins.begin(), saturatedBins.end(), i) != saturatedBins.end())
                continue;
            g->AddPoint(i, (*signal)[i]);
        }
        //g = signal->GetGraph(); // this would return (*signal)[i]-baseLine (if baseline has been initialized)

        for (size_t i = 0; i < saturatedBins.size(); i++) {
            g->RemovePoint(maxPeakBin); // when one point is removed the other points are shifted
        }

 
        // ShaperSin function (AGET theoretic curve) times logistic function. Better without the sin
        TF1* f = new TF1("fit",
                         "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                         "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] / "
                         "(1+TMath::Exp(-10000*(x-[3])))",
                         0, signal->GetNumberOfPoints());
        auto peakPosition = maxPeakBin + saturatedBins.size()/2;
        Double_t amplEstimate = maxValue;
        Double_t widthEstimate = 30;
        auto pOverThreshold = signal->GetPointsOverThreshold();
        if (!pOverThreshold.empty()) {
            amplEstimate = 0.9 * (maxValue - signal->GetRawData(pOverThreshold[0])) / (maxPeakBin-pOverThreshold[0]) * (peakPosition-pOverThreshold[0]);
            if (amplEstimate < maxValue) amplEstimate = maxValue;
        }

        f->SetParameters(250, amplEstimate/0.0498, widthEstimate, peakPosition-widthEstimate);
        // f->SetParameters(0, 250);  // no baseline correction is used in this process
        // f->SetParLimits(0, 150, 350);
        // f->SetParameters(1, 2000);
        // f->SetParLimits(1, 30, 90000);
        // f->SetParameters(2, 70);
        // f->SetParLimits(2, 10, 80);
        // f->SetParameters(3, 80);
        // f->SetParLimits(3, 150, 250);
        f->SetParNames("Baseline", "Amplitude", "ShapingTime", "PeakPosition");
        if (signal->isBaseLineInitialized()){
            f->FixParameter(0, signal->GetBaseLine());
        }

        g->Fit(f, "RNQ");

        // Add the recovered signal to the original signal
        TRestRawSignal toAddSignal(0);
        bool anyBinRecovered = false;
        for (size_t i = 0; i < (size_t)signal->GetNumberOfPoints(); i++) {
            if (std::find(saturatedBins.begin(), saturatedBins.end(), i) != saturatedBins.end()){
                Double_t value = f->Eval(i)-maxValue;
                if (value > 0){
                    toAddSignal.AddPoint(value);
                    anyBinRecovered = true;
                    addedIntegral += value;
                    if (value > addedAmplitude) addedAmplitude = value;
                }
                else toAddSignal.AddPoint((Short_t) 0);
            }
            else {
                toAddSignal.AddPoint((Short_t) 0);
            }
        }
        delete g;
        delete f;

        if (anyBinRecovered) {
            nSignalsRecovered++;
        }
        else {
            RESTDebug << "Signal " << s << " not recovered" << RESTendl;
            continue; // nothing to add
        }
        signal->SignalAddition(toAddSignal);
    }

    SetObservableValue("AddedAmplitude", addedAmplitude);
    SetObservableValue("AddedIntegral", addedIntegral);
    SetObservableValue("SignalsSaturated", nSignalsSaturated);
    SetObservableValue("SignalsRecovered", nSignalsRecovered);
 
    return fAnaEvent;
}

///////////////////////////////////////////////                          
/// \brief Function to include required actions after all events have been
/// processed.                                                            
///                                                                       
void TRestRawSignalRecoverSaturationProcess::EndProcess() {
    // Write here the jobs to do when all the events are processed

}

