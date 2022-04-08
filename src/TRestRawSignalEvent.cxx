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
/// The TRestRawSignalEvent ...
///
/// DOCUMENTATION TO BE WRITTEN (main description, figures, methods, data
/// members)
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
/// 2017-Feb: First concept (inspired by TRestDetectorSignalEvent)
///           Javier Galan
///
/// \class      TRestRawSignalEvent
/// \author     Javier Galan
///
/// <hr>
///

#include "TRestRawSignalEvent.h"

#include <TMath.h>

#include "TRestStringHelper.h"

using namespace std;

ClassImp(TRestRawSignalEvent);

TRestRawSignalEvent::TRestRawSignalEvent() {
    // TRestRawSignalEvent default constructor
    Initialize();
}

TRestRawSignalEvent::~TRestRawSignalEvent() {
    // TRestRawSignalEvent destructor
}

void TRestRawSignalEvent::Initialize() {
    TRestEvent::Initialize();
    fSignal.clear();
    fPad = nullptr;
    mg = nullptr;
    gr = nullptr;
    fMinValue = 1E10;
    fMaxValue = -1E10;
    fMinTime = 1E10;
    fMaxTime = -1E10;
}

void TRestRawSignalEvent::AddSignal(TRestRawSignal& s) {
    if (signalIDExists(s.GetSignalID())) {
        cout << "Warning. Signal ID : " << s.GetSignalID()
             << " already exists. Signal will not be added to signal event" << endl;
        return;
    }

    s.CalculateBaseLine(fBaseLineRange.X(), fBaseLineRange.Y());
    s.SetRange(fRange);

    fSignal.emplace_back(s);
}

void TRestRawSignalEvent::RemoveSignalWithId(Int_t sId) {
    Int_t index = GetSignalIndex(sId);

    if (index == -1) {
        cout << "Warning. Signal ID : " << sId
             << " does not exist. Signal will not be removed from signal event" << endl;
        return;
    }

    fSignal.erase(fSignal.begin() + index);
}

Int_t TRestRawSignalEvent::GetSignalIndex(Int_t signalID) {
    for (int i = 0; i < GetNumberOfSignals(); i++)
        if (fSignal[i].GetSignalID() == signalID) return i;
    return -1;
}

Double_t TRestRawSignalEvent::GetIntegral() {
    Double_t sum = 0;

    for (int i = 0; i < GetNumberOfSignals(); i++) sum += fSignal[i].GetIntegral();

    return sum;
}

/// The result if this method depends on InitializePointsOverThreshold.
/// Arguments are given there.
Double_t TRestRawSignalEvent::GetThresholdIntegral() {
    Double_t sum = 0;
    for (int i = 0; i < GetNumberOfSignals(); i++) sum += fSignal[i].GetThresholdIntegral();
    return sum;
}

TRestRawSignal* TRestRawSignalEvent::GetMaxSignal() {
    if (GetNumberOfSignals() <= 0) return nullptr;

    Double_t max = fSignal[0].GetIntegral();

    Int_t sId;
    for (int i = 0; i < GetNumberOfSignals(); i++) {
        Int_t integ = fSignal[i].GetIntegral();
        if (max < integ) {
            max = integ;
            sId = i;
        }
    }

    return &fSignal[sId];
}

Double_t TRestRawSignalEvent::GetSlopeIntegral() {
    Double_t sum = 0;

    for (int i = 0; i < GetNumberOfSignals(); i++) sum += fSignal[i].GetSlopeIntegral();

    return sum;
}

Double_t TRestRawSignalEvent::GetRiseSlope() {
    Double_t sum = 0;

    Int_t n = 0;
    for (int i = 0; i < GetNumberOfSignals(); i++) {
        if (fSignal[i].GetThresholdIntegral() > 0) {
            sum += fSignal[i].GetSlopeIntegral();
            n++;
        }
    }

    if (n == 0) return 0;

    return sum / n;
}

Double_t TRestRawSignalEvent::GetRiseTime() {
    Double_t sum = 0;

    Int_t n = 0;
    for (int i = 0; i < GetNumberOfSignals(); i++) {
        if (fSignal[i].GetThresholdIntegral() > 0) {
            sum += fSignal[i].GetRiseTime();
            n++;
        }
    }

    if (n == 0) return 0;

    return sum / n;
}

Double_t TRestRawSignalEvent::GetTripleMaxIntegral() {
    Double_t sum = 0;

    for (int i = 0; i < GetNumberOfSignals(); i++)
        if (fSignal[i].GetThresholdIntegral() > 0) sum += fSignal[i].GetTripleMaxIntegral();

    return sum;
}

Double_t TRestRawSignalEvent::GetBaseLineAverage() {
    Double_t baseLineMean = 0;

    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++) {
        Double_t baseline = GetSignal(sgnl)->GetBaseLine();
        baseLineMean += baseline;
    }

    return baseLineMean / GetNumberOfSignals();
}

Int_t TRestRawSignalEvent::GetLowestWidth(Double_t minPeakAmplitude) {
    Int_t low = 10000000;

    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++) {
        if (GetSignal(sgnl)->GetMaxPeakValue() > minPeakAmplitude) {
            Int_t lW = GetSignal(sgnl)->GetMaxPeakWidth();
            if (low > lW) low = lW;
        }
    }

    return low;
}

Double_t TRestRawSignalEvent::GetAverageWidth(Double_t minPeakAmplitude) {
    Double_t avg = 0;
    Int_t n = 0;
    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++) {
        if (GetSignal(sgnl)->GetMaxPeakValue() > minPeakAmplitude) {
            avg += GetSignal(sgnl)->GetMaxPeakWidth();
            n++;
        }
    }

    if (n == 0)
        return 0;
    else
        return avg / n;
}

Double_t TRestRawSignalEvent::GetLowAverageWidth(Int_t nSignals, Double_t minPeakAmplitude) {
    std::vector<Double_t> widths;

    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++)
        if (GetSignal(sgnl)->GetMaxPeakValue() > minPeakAmplitude)
            widths.push_back(GetSignal(sgnl)->GetMaxPeakWidth());

    if (widths.size() == 0) return 0;

    std::sort(widths.begin(), widths.end());

    Int_t nMax = nSignals;
    if (widths.size() < (unsigned int)nSignals) nMax = widths.size();

    Double_t avg = 0;
    for (int n = 0; n < nMax; n++) avg += widths[n];

    return avg / nSignals;
}

Double_t TRestRawSignalEvent::GetBaseLineSigmaAverage() {
    Double_t baseLineSigmaMean = 0;

    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++) {
        Double_t baselineSigma = GetSignal(sgnl)->GetBaseLineSigma();
        baseLineSigmaMean += baselineSigma;
    }

    return baseLineSigmaMean / GetNumberOfSignals();
}

/// Perhaps we should not substract baselines on a TRestRawSignal. Just consider
/// it in the observables
/// calculation if a baseline range is provided in the argument (as it is done
/// in
/// InitializeThresholdIntegral). This method should be probably removed.
// void TRestRawSignalEvent::SubstractBaselines() {
//    for (int sgnl = 0; sgnl < GetNumberOfSignals(); sgnl++)
//    GetSignal(sgnl)->SubstractBaseline();
//}

void TRestRawSignalEvent::AddChargeToSignal(Int_t sgnlID, Int_t bin, Short_t value) {
    Int_t sgnlIndex = GetSignalIndex(sgnlID);
    if (sgnlIndex == -1) {
        sgnlIndex = GetNumberOfSignals();

        TRestRawSignal sgnl(512);  // For the moment we use the default nBins=512
        sgnl.SetSignalID(sgnlID);
        AddSignal(sgnl);
    }

    fSignal[sgnlIndex].IncreaseBinBy(bin, value);
}

void TRestRawSignalEvent::PrintEvent() {
    TRestEvent::PrintEvent();

    for (int i = 0; i < GetNumberOfSignals(); i++) {
        cout << "================================================" << endl;
        cout << "Signal ID : " << fSignal[i].GetSignalID() << endl;
        cout << "Integral : " << fSignal[i].GetIntegral() << endl;
        cout << "------------------------------------------------" << endl;
        fSignal[i].Print();
        cout << "================================================" << endl;
    }
}

// TODO: GetMaxTimeFast, GetMinTimeFast, GetMaxValueFast that return the value
// of fMinTime, fMaxTime, etc
void TRestRawSignalEvent::SetMaxAndMin() {
    fMinValue = 1E10;
    fMaxValue = -1E10;
    fMinTime = 0;
    fMaxTime = -1E10;

    for (int s = 0; s < GetNumberOfSignals(); s++) {
        if (fMinValue > fSignal[s].GetMinValue()) fMinValue = fSignal[s].GetMinValue();
        if (fMaxValue < fSignal[s].GetMaxValue()) fMaxValue = fSignal[s].GetMaxValue();
    }

    if (GetNumberOfSignals() > 0) fMaxTime = fSignal[0].GetNumberOfPoints();
}

Double_t TRestRawSignalEvent::GetMaxValue() {
    SetMaxAndMin();
    return fMaxValue;
}

Double_t TRestRawSignalEvent::GetMinValue() {
    SetMaxAndMin();
    return fMinValue;
}

Double_t TRestRawSignalEvent::GetMinTime() { return 0; }

Double_t TRestRawSignalEvent::GetMaxTime() {
    Double_t maxTime = 512;

    if (GetNumberOfSignals() > 0) maxTime = fSignal[0].GetNumberOfPoints();

    return maxTime;
}

///////////////////////////////////////////////
/// \brief This method draws current raw signal event in a TPad.
///
/// This method receives an optional argument as string that allows to control
/// which signals will be plotted. Different options can be passed to the string
/// by
/// separated the options using colons, as "option1:option2:option3".
///
/// The following options are allowed:
///
/// 1. **from-to**: It imposes that only the signal entries on the specified
///    range will be plotted.
///
/// 4. **ids[startId,endId]** or **signalRangeID[startId,endId]**: It imposes
/// that
///    the signal ids drawn are inside the given range. Giving the range using
///    `-`
///    symbol (as in previous option) is also allowed. I.e. `ids[10-20]`.
///
/// 2. **onlyGoodSignals[pointTh,signalTh,nOver]**: It imposes that only signals
///    where points were identified over the threshold will be plotted. The
///    parameters
///    provided are the parameters given to the method
///    TRestRawSignal::InitializePointsOverThreshold.
///    \warning When using this option **baseLineRange** option must also be
///    defined.
///
/// 3. **baseLineRange[start,end]**: It defines the bin range (start,end) where
/// the baseline
///    will be calculated.
///
/// 5. **printIDs**: Prints by screen the ID of plotted signals.
///
/// If no option is given, all signals will be plotted.
///
///
/// Example 1:
/// \code
/// DrawEvent("0-10:onlyGoodSignals[3.5,1.5,7]:baseLineRange[20,150]:printIDs");
/// \endcode
///
/// Example 2:
/// \code
/// DrawEvent("signalRangeID[800,900]:onlyGoodSignals[3.5,1.5,7]:baseLineRange[20,150]");
/// \endcode
///
/// Example 3:
/// \code
/// DrawEvent("ids[800,900]:printIDs");
/// \endcode
///
TPad* TRestRawSignalEvent::DrawEvent(const TString& option) {
    int nSignals = this->GetNumberOfSignals();

    if (fPad != nullptr) {
        for (auto& signal : fSignal) {
            delete signal.fGraph;
            signal.fGraph = nullptr;
        }
        delete fPad;
        fPad = nullptr;
    }

    if (nSignals == 0) {
        cout << "Empty event " << endl;
        return nullptr;
    }

    fMinValue = 1E10;
    fMaxValue = -1E10;
    fMinTime = 1E10;
    fMaxTime = -1E10;

    fPad = new TPad(this->GetName(), " ", 0, 0, 1, 1);
    fPad->Draw();
    fPad->cd();
    // fPad->DrawFrame(0, GetMinValue() - 1, GetMaxTime() + 1, GetMaxValue() + 1);

    char title[256];
    vector<TString> optList = Vector_cast<string, TString>(TRestTools::GetOptions((string)option));

    bool ThresCheck = false;
    bool BLCheck = false;
    bool sRangeID = false;
    bool printIDs = false;

    double signalTh = 0, pointTh = 0, nOver = 0;
    int baseLineRangeInit = 0, baseLineRangeEnd = 0;
    int sRangeInit = 0, sRangeEnd = 0;

    for (int j = 0; j < optList.size(); j++) {
        string str = (string)optList[j];

        // Read threshold option
        size_t goodSigOpt = str.find("onlyGoodSignals[");

        if (goodSigOpt != string::npos) {
            size_t startPos = str.find("[");
            size_t endPos = str.find("]");
            TString tmpStr = optList[j](startPos + 1, endPos - startPos - 1);
            vector<TString> optList_2 = Vector_cast<string, TString>(Split((string)tmpStr, ","));

            pointTh = StringToDouble((string)optList_2[0]);
            signalTh = StringToDouble((string)optList_2[1]);
            nOver = StringToDouble((string)optList_2[2]);

            ThresCheck = true;
        }

        // Read base line option
        size_t BLOpt = str.find("baseLineRange[");

        if (BLOpt != string::npos) {
            size_t startPos2 = str.find("[");
            size_t endPos2 = str.find("]");
            TString tmpStr2 = optList[j](startPos2 + 1, endPos2 - startPos2 - 1);
            vector<TString> optList_3 = Vector_cast<string, TString>(Split((string)tmpStr2, ","));

            baseLineRangeInit = StringToInteger((string)optList_3[0]);
            baseLineRangeEnd = StringToInteger((string)optList_3[1]);

            BLCheck = true;
        }

        // Read signal range ID option
        size_t sRange = str.find("signalRangeID[");
        size_t sRange2 = str.find("ids[");

        if (sRange != string::npos || sRange2 != string::npos) {
            size_t startPos3 = str.find("[");
            size_t endPos3 = str.find("]");
            TString tmpStr3 = optList[j](startPos3 + 1, endPos3 - startPos3 - 1);
            vector<TString> optList_4;
            if (str.find(",") != string::npos)
                optList_4 = Vector_cast<string, TString>(Split((string)tmpStr3, ","));
            else if (str.find("-") != string::npos)
                optList_4 = Vector_cast<string, TString>(Split((string)tmpStr3, "-"));
            else
                ferr << "TRestRawSignalEvent::DrawEvent not valid ids format!" << endl;

            sRangeInit = StringToInteger((string)optList_4[0]);
            sRangeEnd = StringToInteger((string)optList_4[1]);

            sRangeID = true;
        }

        // Read print ID option
        size_t screenIDs = str.find("printIDs");

        if (screenIDs != string::npos) {
            printIDs = true;
            cout << "IDs of printed signals: " << endl;
        }
    }

    ///// No specific signal selection ////
    if ((optList.size() == 0) || (isANumber((string)optList[0]) == false)) {
        delete mg;
        sprintf(title, "Event ID %d", this->GetID());
        mg = new TMultiGraph();

        int sigPrinted = 0;

        // If threshold and baseline options are given
        if (ThresCheck == true && BLCheck == true) {
            debug << "Draw only good signals with: " << endl;
            debug << "  Signal threshold: " << signalTh << endl;
            debug << "  Point threshold: " << pointTh << endl;
            debug << "  Points over threshold: " << nOver << endl;
            debug << "  Base line range: (" << baseLineRangeInit << "," << baseLineRangeEnd << ")" << endl;

            for (int n = 0; n < nSignals; n++) {
                fSignal[n].CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
                fSignal[n].InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);
                if (fSignal[n].GetPointsOverThreshold().size() >= 2 && sRangeID == false) {
                    TGraph* gr = fSignal[n].GetGraph(n + 1);
                    mg->Add(gr);
                    sigPrinted++;
                    if (printIDs == true) {
                        cout << "id : " << fSignal[n].GetID() << endl;
                    }
                }
                if (fSignal[n].GetPointsOverThreshold().size() >= 2 && sRangeID == true) {
                    if (fSignal[n].GetID() >= sRangeInit && fSignal[n].GetID() <= sRangeEnd) {
                        TGraph* gr = fSignal[n].GetGraph(n + 1);
                        mg->Add(gr);
                        sigPrinted++;
                        if (printIDs == true) {
                            cout << fSignal[n].GetID() << endl;
                        }
                    }
                }
            }
        }
        // If no threshold and baseline options are given
        else {
            for (int n = 0; n < nSignals; n++) {
                if (sRangeID == false) {
                    TGraph* gr = fSignal[n].GetGraph(n + 1);
                    mg->Add(gr);
                    sigPrinted++;
                    if (printIDs == true) {
                        cout << fSignal[n].GetID() << endl;
                    }
                }
                if (sRangeID == true) {
                    if (fSignal[n].GetID() >= sRangeInit && fSignal[n].GetID() <= sRangeEnd) {
                        TGraph* gr = fSignal[n].GetGraph(n + 1);
                        mg->Add(gr);
                        sigPrinted++;
                        if (printIDs == true) {
                            cout << fSignal[n].GetID() << endl;
                        }
                    }
                }
            }
        }
        cout << "Number of drawn signals: " << sigPrinted << endl;

        fPad->cd();
        fPad->DrawFrame(0, GetMinValue() - 1, GetMaxTime() + 1, GetMaxValue() + 1);
        mg->Draw("");
        mg->SetTitle(title);
        mg->GetXaxis()->SetTitle("time bins");
        mg->GetYaxis()->SetTitleOffset(1.4);
        mg->GetYaxis()->SetTitle("Amplitude [ADC units]");
    }

    //// Signal selection (range or sigle signal) ////
    else if (isANumber((string)optList[0]) == true) {
        string str = (string)optList[0];
        size_t separation = str.find("-");

        // Signals range //
        if (separation != string::npos) {
            TString firstSignal = optList[0](0, separation);
            TString lastSignal = optList[0](separation + 1, str.size());
            debug << "First signal: " << firstSignal << endl;
            debug << "Last signal: " << lastSignal << endl;

            if (StringToInteger((string)lastSignal) >= fSignal.size()) {
                fPad->SetTitle("No Such Signal");
                cout << "No such last signal" << endl;
                return fPad;
            }

            delete mg;
            mg = new TMultiGraph();
            sprintf(title, "Event ID %d", this->GetID());

            if (ThresCheck == true && BLCheck == true) {
                debug << "Draw only good signals with: " << endl;
                debug << "  Signal threshold: " << signalTh << endl;
                debug << "  Point threshold: " << pointTh << endl;
                debug << "  Points over threshold: " << nOver << endl;
                debug << "  Base line range: (" << baseLineRangeInit << "," << baseLineRangeEnd << ")"
                      << endl;

                int sigPrinted = 0;
                for (int n = 0; n < nSignals; n++) {
                    fSignal[n].CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
                    fSignal[n].InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);
                    if (fSignal[n].GetPointsOverThreshold().size() >= 2 &&
                        n >= StringToInteger((string)firstSignal) &&
                        n <= StringToInteger((string)lastSignal)) {
                        TGraph* gr = fSignal[n].GetGraph(n + 1);
                        mg->Add(gr);
                        if (fMinValue > fSignal[n].GetMinValue()) fMinValue = fSignal[n].GetMinValue();
                        if (fMaxValue < fSignal[n].GetMaxValue()) fMaxValue = fSignal[n].GetMaxValue();
                        sigPrinted++;
                        if (printIDs == true) {
                            cout << fSignal[n].GetID() << endl;
                        }
                    }
                }
                cout << "Number of good signals in range (" << firstSignal << "," << lastSignal
                     << "): " << sigPrinted << endl;
            }
            // If no threshold and baseline options are given
            else {
                for (int n = StringToInteger((string)firstSignal);
                     n < StringToInteger((string)lastSignal) + 1; n++) {
                    TGraph* gr = fSignal[n].GetGraph(n + 1);
                    mg->Add(gr);
                    if (printIDs == true) {
                        cout << fSignal[n].GetID() << endl;
                    }
                    if (fMinValue > fSignal[n].GetMinValue()) fMinValue = fSignal[n].GetMinValue();
                    if (fMaxValue < fSignal[n].GetMaxValue()) fMaxValue = fSignal[n].GetMaxValue();
                }
            }
            fPad->cd();
            fPad->DrawFrame(0, fMinValue - 1, GetMaxTime() + 1, fMaxValue + 1);
            mg->SetTitle(title);
            mg->Draw("");
            mg->GetXaxis()->SetTitle("time bins");
            mg->GetYaxis()->SetTitleOffset(1.4);
            mg->GetYaxis()->SetTitle("Amplitude [ADC units]");
        }

        // Single signal //
        if (separation == string::npos) {
            int signalid = StringToInteger((string)optList[0]);

            if (signalid >= fSignal.size()) {
                fPad->SetTitle("No Such Signal");
                cout << "No such signal" << endl;
                return fPad;
            }
            TRestRawSignal& sgn = fSignal[signalid];

            sprintf(title, "Event ID %d, Signal ID. %d", this->GetID(), sgn.GetID());

            gr = sgn.GetGraph(1);
            gr->SetTitle(title);
            gr->GetXaxis()->SetTitle("time bins");
            gr->GetYaxis()->SetTitleOffset(1.4);
            gr->GetYaxis()->SetTitle("Amplitude [a.u.]");

            fPad->cd();
            gr->Draw("ALP");
        }
    }

    return fPad;
}

///////////////////////////////////////////////
/// \brief This method draws selected signal IDs, given by the vector 
/// passed as reference
///

TPad* TRestRawSignalEvent::DrawSignals(const std::vector<Int_t> &signals) {

  int maxSID=-1;
  int max=0;
  int color=1;

  for(const auto &s : signals){
    TRestRawSignal* sgnl = GetSignalById(s);
    if(sgnl->fGraph)delete sgnl->fGraph;
    sgnl->fGraph = new TGraph();
      for(int p=0;p<sgnl->GetNumberOfPoints();p++){
        double val = sgnl->GetData(p);
        sgnl->fGraph->SetPoint(p,p,val);
        sgnl->fGraph->SetLineColor(color);
        if(val>max){max=val;maxSID=s;};
      }
     color++;
  }

  TPad *pad =new TPad("RawSignal", " ", 0, 0, 1, 1);
  pad->Draw();

  debug<<"Max SID "<<maxSID<<endl;

  if(maxSID ==-1)return pad;

  TRestRawSignal* sgn = GetSignalById(maxSID);
  pad->cd();
  sgn->fGraph->Draw();

  for(const auto &s : signals){
    TRestRawSignal* sgnl = GetSignalById(s);
    if(sgnl->GetID() !=sgn->GetID())sgnl->fGraph->Draw("SAME"); 
  }

return pad;
}

///////////////////////////////////////////////
/// \brief This method draws selected signal by ID, with baseline range and
/// points over threshold highlighted.
///
/// In order to compute points over threshold the following parameters should
/// be provided:
///
/// **goodSignals[pointTh,signalTh,nOver]**: These parameters are the ones needed
/// to call the method TRestRawSignal::InitializePointsOverThreshold.
///    \warning When using this option **baseLineRange** option must also be
///    defined.
///
/// **baseLineRange[start,end]**: It defines the bin range (start,end) where
/// the baseline will be calculated.
///
/// Example 1:
/// \code
/// DrawEvent(100,"goodSignals[3.5,1.5,7]:baseLineRange[20,150]");
/// \endcode
///
TPad* TRestRawSignalEvent::DrawSignal(Int_t signal, TString option) {
    int nSignals = this->GetNumberOfSignals();

    if (fPad != nullptr) {
        for (int n = 0; n < nSignals; n++) {
            delete fSignal[n].fGraph;
            fSignal[n].fGraph = nullptr;
        }
        delete fPad;
        fPad = nullptr;
    }

    if (nSignals == 0) {
        cout << "Empty event " << endl;
        return nullptr;
    }

    vector<TString> optList = Vector_cast<string, TString>(TRestTools::GetOptions((string)option));

    bool ThresCheck = false;
    bool BLCheck = false;

    double signalTh = 0, pointTh = 0, nOver = 0;
    int baseLineRangeInit = 0, baseLineRangeEnd = 0;

    for (int j = 0; j < optList.size(); j++) {
        string str = (string)optList[j];

        // Read threshold option
        size_t goodSigOpt = str.find("goodSignals[");

        if (goodSigOpt != string::npos) {
            size_t startPos = str.find("[");
            size_t endPos = str.find("]");
            TString tmpStr = optList[j](startPos + 1, endPos - startPos - 1);
            vector<TString> optList_2 = Vector_cast<string, TString>(Split((string)tmpStr, ","));

            pointTh = StringToDouble((string)optList_2[0]);
            signalTh = StringToDouble((string)optList_2[1]);
            nOver = StringToDouble((string)optList_2[2]);

            ThresCheck = true;
        }

        // Read base line option
        size_t BLOpt = str.find("baseLineRange[");

        if (BLOpt != string::npos) {
            size_t startPos2 = str.find("[");
            size_t endPos2 = str.find("]");
            TString tmpStr2 = optList[j](startPos2 + 1, endPos2 - startPos2 - 1);
            vector<TString> optList_3 = Vector_cast<string, TString>(Split((string)tmpStr2, ","));

            baseLineRangeInit = StringToInteger((string)optList_3[0]);
            baseLineRangeEnd = StringToInteger((string)optList_3[1]);

            BLCheck = true;
        }
    }

    fPad = new TPad(this->GetName(), " ", 0, 0, 1, 1);
    fPad->Draw();
    fPad->cd();

    TGraph* gr = new TGraph();

    TRestRawSignal* sgnl = this->GetSignalById(signal);
    sgnl->CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
    sgnl->InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);

    info << "Drawing signal. Event ID : " << this->GetID() << " Signal ID : " << sgnl->GetID() << endl;

    for (int n = 0; n < sgnl->GetNumberOfPoints(); n++) gr->SetPoint(n, n, sgnl->GetData(n));

    gr->Draw("AC*");

    TGraph* gr2 = new TGraph();

    gr2->SetLineWidth(2);
    gr2->SetLineColor(2);  // Red

    for (int n = baseLineRangeInit; n < baseLineRangeEnd; n++)
        gr2->SetPoint(n - baseLineRangeInit, n, sgnl->GetData(n));

    gr2->Draw("CP");

    vector<Int_t> pOver = sgnl->GetPointsOverThreshold();

    TGraph* gr3[5];
    Int_t nGraphs = 0;
    gr3[nGraphs] = new TGraph();
    gr3[nGraphs]->SetLineWidth(2);
    gr3[nGraphs]->SetLineColor(3);
    Int_t point = 0;
    Int_t nPoints = pOver.size();
    for (int n = 0; n < nPoints; n++) {
        gr3[nGraphs]->SetPoint(point, pOver[n], sgnl->GetData(pOver[n]));
        point++;
        if (n + 1 < nPoints && pOver[n + 1] - pOver[n] > 1) {
            gr3[nGraphs]->Draw("CP");
            nGraphs++;
            if (nGraphs > 4) cout << "Ngraphs : " << nGraphs << endl;
            point = 0;
            gr3[nGraphs] = new TGraph();
            gr3[nGraphs]->SetLineWidth(2);
            gr3[nGraphs]->SetLineColor(3);  // Green
        }
    }

    if (nPoints > 0) gr3[nGraphs]->Draw("CP");

    return fPad;
}
