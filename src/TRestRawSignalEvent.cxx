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
/// \htmlonly <style>div.image img[src="rawsignals.png"]{width:750px;}</style> \endhtmlonly
///
/// ![Raw event signals produced with DrawEvent method](rawsignals.png)
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
    gr = nullptr;

    fMinValue = numeric_limits<Double_t>::max();
    fMaxValue = numeric_limits<Double_t>::min();
    fMinTime = numeric_limits<Double_t>::max();
    fMaxTime = numeric_limits<Double_t>::min();
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

    Int_t sId = 0;
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

    for (int signal = 0; signal < GetNumberOfSignals(); signal++) {
        Double_t baseline = GetSignal(signal)->GetBaseLine();
        baseLineMean += baseline;
    }

    return baseLineMean / GetNumberOfSignals();
}

Int_t TRestRawSignalEvent::GetLowestWidth(Double_t minPeakAmplitude) {
    Int_t low = 10000000;

    for (int signal = 0; signal < GetNumberOfSignals(); signal++) {
        if (GetSignal(signal)->GetMaxPeakValue() > minPeakAmplitude) {
            Int_t lW = GetSignal(signal)->GetMaxPeakWidth();
            if (low > lW) low = lW;
        }
    }

    return low;
}

Double_t TRestRawSignalEvent::GetAverageWidth(Double_t minPeakAmplitude) {
    Double_t avg = 0;
    Int_t n = 0;
    for (int signal = 0; signal < GetNumberOfSignals(); signal++) {
        if (GetSignal(signal)->GetMaxPeakValue() > minPeakAmplitude) {
            avg += GetSignal(signal)->GetMaxPeakWidth();
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

    for (int signal = 0; signal < GetNumberOfSignals(); signal++)
        if (GetSignal(signal)->GetMaxPeakValue() > minPeakAmplitude)
            widths.push_back(GetSignal(signal)->GetMaxPeakWidth());

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

    for (int signal = 0; signal < GetNumberOfSignals(); signal++) {
        Double_t baselineSigma = GetSignal(signal)->GetBaseLineSigma();
        baseLineSigmaMean += baselineSigma;
    }

    return baseLineSigmaMean / GetNumberOfSignals();
}

/// Perhaps we should not subtract baselines on a TRestRawSignal. Just consider
/// it in the observables
/// calculation if a baseline range is provided in the argument (as it is done
/// in
/// InitializeThresholdIntegral). This method should be probably removed.
// void TRestRawSignalEvent::subtractBaselines() {
//    for (int signal = 0; signal < GetNumberOfSignals(); signal++)
//    GetSignal(signal)->subtractBaseline();
//}

void TRestRawSignalEvent::AddChargeToSignal(Int_t signalID, Int_t bin, Short_t value) {
    Int_t signalIndex = GetSignalIndex(signalID);
    if (signalIndex == -1) {
        signalIndex = GetNumberOfSignals();

        TRestRawSignal signal(512);  // For the moment we use the default nBins=512
        signal.SetSignalID(signalID);
        AddSignal(signal);
    }

    fSignal[signalIndex].IncreaseBinBy(bin, value);
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
    fMinValue = numeric_limits<Double_t>::max();
    fMaxValue = numeric_limits<Double_t>::min();
    fMinTime = numeric_limits<Double_t>::max();
    fMaxTime = numeric_limits<Double_t>::min();

    for (int s = 0; s < GetNumberOfSignals(); s++) {
        if (fMinValue > fSignal[s].GetMinValue()) {
            fMinValue = fSignal[s].GetMinValue();
        }
        if (fMaxValue < fSignal[s].GetMaxValue()) {
            fMaxValue = fSignal[s].GetMaxValue();
        }
    }

    if (GetNumberOfSignals() > 0) {
        fMaxTime = fSignal[0].GetNumberOfPoints();
    }
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
    const int nSignals = GetNumberOfSignals();

    if (fPad != nullptr) {
        delete fPad;
        fPad = nullptr;
    }

    if (nSignals == 0) {
        cout << "Empty event " << endl;
        return nullptr;
    }

    fMinValue = numeric_limits<Double_t>::max();
    fMaxValue = numeric_limits<Double_t>::min();
    fMinTime = numeric_limits<Double_t>::max();
    fMaxTime = numeric_limits<Double_t>::min();

    fPad = new TPad(GetName(), " ", 0, 0, 1, 1);
    fPad->Draw();
    fPad->cd();
    // fPad->DrawFrame(0, GetMinValue() - 1, GetMaxTime() + 1, GetMaxValue() + 1);

    char title[256];
    vector<TString> optList = Vector_cast<string, TString>(TRestTools::GetOptions((string)option));

    bool thresholdCheck = false;
    bool baselineCheck = false;
    bool sRangeID = false;
    bool printIDs = false;

    double signalTh = 0, pointTh = 0, nOver = 0;
    int baseLineRangeInit = 0, baseLineRangeEnd = 0;
    int sRangeInit = 0, sRangeEnd = 0;

    for (const auto& opt : optList) {
        std::string str = (std::string)opt;
        // Read threshold option
        if (str.find("onlyGoodSignals[") != string::npos) {
            size_t startPos = str.find('[');
            size_t endPos = str.find(']');
            TString tmpStr = opt(startPos + 1, endPos - startPos - 1);
            vector<TString> optList_2 = Vector_cast<string, TString>(Split((string)tmpStr, ","));

            pointTh = StringToDouble((string)optList_2[0]);
            signalTh = StringToDouble((string)optList_2[1]);
            nOver = StringToDouble((string)optList_2[2]);

            thresholdCheck = true;
        }

        // Read baseline option
        if (str.find("baseLineRange[") != string::npos) {
            size_t startPos2 = str.find('[');
            size_t endPos2 = str.find(']');
            TString tmpStr2 = opt(startPos2 + 1, endPos2 - startPos2 - 1);
            vector<TString> optList_3 = Vector_cast<string, TString>(Split((string)tmpStr2, ","));

            baseLineRangeInit = StringToInteger((string)optList_3[0]);
            baseLineRangeEnd = StringToInteger((string)optList_3[1]);

            baselineCheck = true;
        }

        // Read signal range ID option
        if (str.find("signalRangeID[") != string::npos || str.find("ids[") != string::npos) {
            size_t startPos3 = str.find('[');
            size_t endPos3 = str.find(']');
            TString tmpStr3 = opt(startPos3 + 1, endPos3 - startPos3 - 1);
            vector<TString> optList_4;
            if (str.find(',') != string::npos)
                optList_4 = Vector_cast<string, TString>(Split((string)tmpStr3, ","));
            else if (str.find('-') != string::npos)
                optList_4 = Vector_cast<string, TString>(Split((string)tmpStr3, "-"));
            else
                RESTError << "TRestRawSignalEvent::DrawEvent not valid ids format!" << RESTendl;

            sRangeInit = StringToInteger((string)optList_4[0]);
            sRangeEnd = StringToInteger((string)optList_4[1]);

            sRangeID = true;
        }

        // Read print ID option
        if (str.find("printIDs") != string::npos) {
            printIDs = true;
            cout << "IDs of printed signals: " << endl;
        }
    }

    std::vector<int> signalIDs;  // Signal IDs to print

    ///// No specific signal selection ////
    if ((optList.empty()) || !(isANumber((string)optList[0]))) {
        // If threshold and baseline options are given
        if (thresholdCheck && baselineCheck) {
            RESTDebug << "Draw only good signals with: " << RESTendl;
            RESTDebug << "  Signal threshold: " << signalTh << RESTendl;
            RESTDebug << "  Point threshold: " << pointTh << RESTendl;
            RESTDebug << "  Points over threshold: " << nOver << RESTendl;
            RESTDebug << "  Base line range: (" << baseLineRangeInit << "," << baseLineRangeEnd << ")"
                      << RESTendl;

            for (int n = 0; n < nSignals; n++) {
                fSignal[n].CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
                fSignal[n].InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);
                if (fSignal[n].GetPointsOverThreshold().size() >= 2) {
                    signalIDs.push_back(fSignal[n].GetID());
                }
            }
            // If no threshold and baseline options are given
        } else {
            for (int n = 0; n < nSignals; n++) {
                signalIDs.push_back(fSignal[n].GetID());
            }
        }

        // Remove SIDs which are not in range
        if (sRangeID) {
            for (auto it = signalIDs.begin(); it != signalIDs.end();)
                if (*it >= sRangeInit && *it <= sRangeEnd) {
                    ++it;
                } else {
                    it = signalIDs.erase(it);
                }
        }

        cout << "Number of drawn signals: " << signalIDs.size() << endl;

        //// Signal selection (range or single signal) ////
    } else if (isANumber((string)optList[0])) {
        string str = (string)optList[0];
        size_t separation = str.find('-');

        // Signals range //
        if (separation != string::npos) {
            TString firstSignal = optList[0](0, separation);
            TString lastSignal = optList[0](separation + 1, str.size());
            RESTDebug << "First signal: " << firstSignal << RESTendl;
            RESTDebug << "Last signal: " << lastSignal << RESTendl;

            if (StringToInteger((string)lastSignal) >= (int)fSignal.size()) {
                fPad->SetTitle("No Such Signal");
                cout << "No such last signal" << endl;
                return fPad;
            }

            sprintf(title, "Event ID %d", this->GetID());

            if (thresholdCheck && baselineCheck) {
                RESTDebug << "Draw only good signals with: " << RESTendl;
                RESTDebug << "  Signal threshold: " << signalTh << RESTendl;
                RESTDebug << "  Point threshold: " << pointTh << RESTendl;
                RESTDebug << "  Points over threshold: " << nOver << RESTendl;
                RESTDebug << "  Base line range: (" << baseLineRangeInit << "," << baseLineRangeEnd << ")"
                          << RESTendl;

                for (int n = 0; n < nSignals; n++) {
                    if (n < StringToInteger((string)firstSignal) || n > StringToInteger((string)lastSignal))
                        continue;
                    fSignal[n].CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
                    fSignal[n].InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);
                    if (fSignal[n].GetPointsOverThreshold().size() >= 2) {
                        signalIDs.push_back(fSignal[n].GetID());
                    }
                }
                cout << "Number of good signals in range (" << firstSignal << "," << lastSignal
                     << "): " << signalIDs.size() << endl;
                // If no threshold and baseline options are given
            } else {
                for (int n = StringToInteger((string)firstSignal);
                     n < StringToInteger((string)lastSignal) + 1; n++) {
                    signalIDs.push_back(fSignal[n].GetID());
                }
            }
            // Single signal
        } else {
            int signalID = StringToInteger((string)optList[0]);
            signalIDs.push_back(signalID);
        }
    }

    if (signalIDs.empty()) {
        fPad->SetTitle("No Such Signal");
        cout << "No signals found" << endl;
        return fPad;
    }

    if (printIDs) {
        cout << "SignalIDs:";
        auto sortedSignalsIDs = signalIDs;
        sort(sortedSignalsIDs.begin(), sortedSignalsIDs.end());
        for (const auto& signalID : sortedSignalsIDs) {
            cout << " " << signalID;
        }
        cout << endl;
    }

    DrawSignals(fPad, signalIDs);

    return fPad;
}

///////////////////////////////////////////////
/// \brief This method draws selected signal IDs, given by the vector
/// passed as reference
///
void TRestRawSignalEvent::DrawSignals(TPad* pad, const std::vector<Int_t>& signals) {
    int maxSID = -1;
    int max = numeric_limits<Short_t>::min();
    int graphIndex = 1;

    for (const auto& s : signals) {
        TRestRawSignal* signal = GetSignalById(s);
        if (!signal) {
            continue;
        }
        TGraph* graph = signal->GetGraph(graphIndex++);
        const double maxValue = TMath::MaxElement(graph->GetN(), graph->GetY());
        if (maxValue > max) {
            max = maxValue;
            maxSID = s;
        }
    }

    RESTDebug << "Max SID " << maxSID << RESTendl;

    if (maxSID == -1) {
        cout << "No signals ID found" << endl;
        return;
    }

    TRestRawSignal* signalMaxID = GetSignalById(maxSID);
    std::string title = "Event ID " + std::to_string(GetID());
    if (signals.size() == 1) {
        title += " Signal ID " + std::to_string(maxSID);
    }

    signalMaxID->fGraph->SetTitle(title.c_str());
    signalMaxID->fGraph->GetXaxis()->SetTitle("Time bin");
    signalMaxID->fGraph->GetYaxis()->SetTitleOffset(1.4);
    signalMaxID->fGraph->GetYaxis()->SetTitle("Amplitude [a.u.]");
    pad->Draw();
    pad->cd();
    signalMaxID->fGraph->SetLineColor(kBlack);
    signalMaxID->fGraph->Draw("AL");

    for (const auto& signalID : signals) {
        if (signalID == maxSID) {
            continue;
        }
        TRestRawSignal* signal = GetSignalById(signalID);
        pad->cd();
        signal->fGraph->Draw("L");
    }

    pad->Update();
}

///////////////////////////////////////////////
/// \brief This method draws selected signalID by ID, with baseline range and
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
TPad* TRestRawSignalEvent::DrawSignal(Int_t signalID, const TString& option) {
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

    double signalTh = 0, pointTh = 0, nOver = 0;
    int baseLineRangeInit = 0, baseLineRangeEnd = 0;

    for (auto& opt : optList) {
        string str = (string)opt;

        // Read threshold option
        size_t goodSigOpt = str.find("goodSignals[");

        if (goodSigOpt != string::npos) {
            size_t startPos = str.find('[');
            size_t endPos = str.find(']');
            TString tmpStr = opt(startPos + 1, endPos - startPos - 1);
            vector<TString> optList_2 = Vector_cast<string, TString>(Split((string)tmpStr, ","));

            pointTh = StringToDouble((string)optList_2[0]);
            signalTh = StringToDouble((string)optList_2[1]);
            nOver = StringToDouble((string)optList_2[2]);
        }

        // Read baseline option
        size_t BLOpt = str.find("baseLineRange[");

        if (BLOpt != string::npos) {
            size_t startPos2 = str.find('[');
            size_t endPos2 = str.find(']');
            TString tmpStr2 = opt(startPos2 + 1, endPos2 - startPos2 - 1);
            vector<TString> optList_3 = Vector_cast<string, TString>(Split((string)tmpStr2, ","));

            baseLineRangeInit = StringToInteger((string)optList_3[0]);
            baseLineRangeEnd = StringToInteger((string)optList_3[1]);
        }
    }

    fPad = new TPad(this->GetName(), " ", 0, 0, 1, 1);
    fPad->Draw();
    fPad->cd();

    TGraph* gr = new TGraph();

    TRestRawSignal* signal = this->GetSignalById(signalID);
    signal->CalculateBaseLine(baseLineRangeInit, baseLineRangeEnd);
    signal->InitializePointsOverThreshold(TVector2(pointTh, signalTh), nOver);

    RESTInfo << "Drawing signalID. Event ID : " << this->GetID() << " Signal ID : " << signal->GetID()
             << RESTendl;

    for (int n = 0; n < signal->GetNumberOfPoints(); n++) {
        gr->SetPoint(n, n, signal->GetData(n));
    }

    gr->Draw("AC*");

    TGraph* gr2 = new TGraph();

    gr2->SetLineWidth(2);
    gr2->SetLineColor(2);  // Red

    for (int n = baseLineRangeInit; n < baseLineRangeEnd; n++) {
        gr2->SetPoint(n - baseLineRangeInit, n, signal->GetData(n));
    }

    gr2->Draw("CP");

    vector<Int_t> pOver = signal->GetPointsOverThreshold();

    TGraph* gr3[5];
    Int_t nGraphs = 0;
    gr3[nGraphs] = new TGraph();
    gr3[nGraphs]->SetLineWidth(2);
    gr3[nGraphs]->SetLineColor(3);
    Int_t point = 0;
    Int_t nPoints = pOver.size();
    for (int n = 0; n < nPoints; n++) {
        gr3[nGraphs]->SetPoint(point, pOver[n], signal->GetData(pOver[n]));
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

    if (nPoints > 0) {
        gr3[nGraphs]->Draw("CP");
    }

    return fPad;
}

TRestRawReadoutMetadata* TRestRawSignalEvent::GetReadoutMetadata() const {
    if (fRun == nullptr) {
        RESTError << "TRestRawSignalEvent::GetReadoutMetadata: fRun is nullptr" << RESTendl;
        return nullptr;
    }
    return dynamic_cast<TRestRawReadoutMetadata*>(fRun->GetMetadataClass("TRestRawReadoutMetadata"));
}

TRestRawSignalEvent TRestRawSignalEvent::GetSignalEventForType(const string& type) const {
    return GetSignalEventForTypes({type});
}

TRestRawSignalEvent TRestRawSignalEvent::GetSignalEventForTypes(
    const std::set<std::string>& types, const TRestRawReadoutMetadata* readoutMetadata ) const {
    TRestRawSignalEvent signalEvent;
    signalEvent.SetEventInfo((TRestEvent*)this);
    auto metadata = readoutMetadata ? readoutMetadata : GetReadoutMetadata();
    if (metadata == nullptr) {
        cerr << "TRestRawSignalEvent::GetSignalEventForTypes: metadata is nullptr" << endl;
        exit(1);
    }
    for (const auto& signal : fSignal) {
        const string signalType = metadata->GetTypeForChannelId(signal.GetSignalID());
        if (types.empty() || types.find(signalType) != types.end()) {
            signalEvent.AddSignal(const_cast<TRestRawSignal&>(signal));
        }
    }
    return signalEvent;
}
