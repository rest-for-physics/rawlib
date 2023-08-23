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
/// TRestRawSignal is ... a longer description comes here
///
/// DOCUMENTATION TO BE WRITTEN (main description, figures, ...)
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find will using it at [The REST Framework
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
/// 2017-February: First concept and implementation of TRestRawSignal class.
/// \author     Javier Galan
///
///	2022-January: Added robust baseline calculation methods
/// \author		Konrad Altenmüller
///
/// \class TRestRawSignal
///
/// <hr>
///
#include "TRestRawSignal.h"

#include <TAxis.h>
#include <TF1.h>
#include <TMath.h>
#include <TRandom3.h>

#include <numeric>

using namespace std;

ClassImp(TRestRawSignal);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestRawSignal::TRestRawSignal() {
    fGraph = nullptr;

    Initialize();
}

///////////////////////////////////////////////
/// \brief Default constructor initializing fSignalData with a number of points
/// equal to nBins.
///
TRestRawSignal::TRestRawSignal(Int_t nBins) {
    fGraph = nullptr;

    Initialize();

    fSignalData.resize(nBins, 0);
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestRawSignal::~TRestRawSignal() {}

///////////////////////////////////////////////
/// \brief Initialization of TRestRawSignal members
///
void TRestRawSignal::Initialize() {
    fSignalData.clear();
    fPointsOverThreshold.clear();
    fSignalID = -1;

    fThresholdIntegral = -1;

    fHeadPoints = 0;
    fTailPoints = 0;

    fBaseLine = 0;
    fBaseLineSigma = 0;
}

///////////////////////////////////////////////
/// \brief Initializes the existing signal data and sets it to zero while
/// keeping the array size.
///
void TRestRawSignal::Reset() {
    Int_t nBins = GetNumberOfPoints();
    Initialize();
    fSignalData.resize(nBins, 0);
}

///////////////////////////////////////////////
/// \brief Adds a new point to the end of the signal data array
///
void TRestRawSignal::AddPoint(Short_t value) { fSignalData.push_back(value); }

///////////////////////////////////////////////
/// \brief Adds a new point to the end of the signal data array
///
void TRestRawSignal::AddPoint(Double_t value) {
    if (value > numeric_limits<Short_t>::max()) {
        value = numeric_limits<Short_t>::max();
    } else if (value < numeric_limits<Short_t>::min()) {
        value = numeric_limits<Short_t>::min();
    }
    AddPoint((Short_t)value);
}

///////////////////////////////////////////////
/// \brief It overloads the operator [] so that we can retrieve a particular
/// point *n* in the form
/// rawSignal[n].
///
Short_t TRestRawSignal::operator[](size_t n) {
    if (n >= GetNumberOfPoints()) {
        if (fShowWarnings) {
            std::cout << "TRestRawSignal::GetSignalData: outside limits" << endl;
            std::cout << "Warnings at TRestRawSignal have been disabled" << endl;
            fShowWarnings = false;
        }
        return 0xFFFF;
    }
    return fSignalData[n];
}

///////////////////////////////////////////////
/// \brief It returns the data value of point *n* including baseline correction.
///
/// This method will substract the internal value of fBaseLine that is extracted
/// from the existing data points
/// after calling the method CalculateBaseLine. If CalculateBaseLine has not
/// been called previously, this
/// method will return the raw values inside fSignalData.
///
Double_t TRestRawSignal::GetData(Int_t n) const { return (Double_t)fSignalData[n] - fBaseLine; }

///////////////////////////////////////////////
/// \brief It returns the original data value of point *n* without baseline
/// correction.
///
Double_t TRestRawSignal::GetRawData(Int_t n) const { return (Double_t)fSignalData[n]; }

///////////////////////////////////////////////
/// \brief It adds the content of data to fSignalData[bin].
///
void TRestRawSignal::IncreaseBinBy(Int_t bin, Double_t data) {
    if (bin >= Int_t(GetNumberOfPoints())) {
        if (fShowWarnings) {
            std::cout << "TRestRawSignal::IncreaseBinBy: outside limits" << endl;
            std::cout << "Warnings at TRestRawSignal have been disabled" << endl;
            fShowWarnings = false;
        }

        return;
    }

    fSignalData[bin] += data;
}

///////////////////////////////////////////////
/// \brief It initializes the fPointsOverThreshold array with the indexes of
/// data points that are found over
/// threshold. The parameters provided to this method are used to identify those
/// points.
///
/// \param thrPar A TVector2 defining two parameters: *pointThreshold* and
/// *signalThreshold*. Both numbers
/// define the number of sigmas over the baseline fluctuation, stored in
/// fBaseLineSigma. The first parameter,
/// *pointThreshold*, serves to identify if a single point is over threshold by
/// satisfying the condition that
/// is above the baseline by the number of sigmas given in *pointThreshold*.
/// Once a certain number of
/// consecutive points have been identified, the parameter *signalThreshold*
/// will serve to reject the signals
/// (consecutive points over threshold) that their standard deviation is lower
/// that *signalThreshold* times
/// the baseline fluctuation.
///
/// \param nPointsOver Only data points with at least *nPointsOver* consecutive
/// points will be considered.
///
/// \param nPointsFlat It will serve to terminate the points over threshold
/// identification in signals where
/// we find an overshoot, being the baseline not returning to zero (or its
/// original value) at the signal tail.
///
void TRestRawSignal::InitializePointsOverThreshold(const TVector2& thrPar, Int_t nPointsOver,
                                                   Int_t nPointsFlat) {
    if (fRange.X() < 0) fRange.SetX(0);
    if (fRange.Y() <= 0) fRange.SetY(GetNumberOfPoints());

    fPointsOverThreshold.clear();

    double pointTh = thrPar.X();
    double signalTh = thrPar.Y();

    double threshold = pointTh * fBaseLineSigma;

    for (int i = fRange.X(); i < fRange.Y(); i++) {
        // Filling a pulse with consecutive points that are over threshold
        if (this->GetData(i) > threshold) {
            int pos = i;
            std::vector<double> pulse;
            pulse.push_back(this->GetData(i));
            i++;

            // If the pulse ends in a flat end above the threshold, the parameter
            // nPointsFlat will serve to artificially end the pulse.
            // If nPointsFlat is big enough, this parameter will not affect the
            // decision to cut this anomalous behaviour. And all points over threshold
            // will be added to the pulse vector.
            int flatN = 0;
            while (i < fRange.Y() && this->GetData(i) > threshold) {
                if (TMath::Abs(this->GetData(i) - this->GetData(i - 1)) > threshold) {
                    flatN = 0;
                } else {
                    flatN++;
                }

                if (flatN < nPointsFlat) {
                    pulse.push_back(this->GetData(i));
                    i++;
                } else {
                    break;
                }
            }

            if (pulse.size() >= (unsigned int)nPointsOver) {
                // auto stdev = TMath::StdDev(begin(pulse), end(pulse));
                // calculate stdev
                double mean = std::accumulate(pulse.begin(), pulse.end(), 0.0) / pulse.size();
                double sq_sum = std::inner_product(pulse.begin(), pulse.end(), pulse.begin(), 0.0);
                double stdev = std::sqrt(sq_sum / pulse.size() - mean * mean);

                if (stdev > signalTh * fBaseLineSigma)
                    for (int j = pos; j < i; j++) fPointsOverThreshold.push_back(j);
            }
        }
    }

    CalculateThresholdIntegral();
}

///////////////////////////////////////////////
/// \brief This method will be called each time InitializePointsOverThreshold is
/// called to re-define the value
/// of fThresholdIntegral. This method is only used internally.
///
void TRestRawSignal::CalculateThresholdIntegral() {
    if (fRange.X() < 0) fRange.SetX(0);
    if (fRange.Y() <= 0 || fRange.Y() > GetNumberOfPoints()) fRange.SetY(GetNumberOfPoints());

    fThresholdIntegral = 0;

    for (unsigned int n = 0; n < fPointsOverThreshold.size(); n++) {
        if (fPointsOverThreshold[n] >= fRange.X() && fPointsOverThreshold[n] < fRange.Y()) {
            fThresholdIntegral += GetData(fPointsOverThreshold[n]);
        }
    }
}

///////////////////////////////////////////////
/// \brief It returns the integral of points found in the region defined by
/// fRange. If fRange was not defined
/// the integral is calculated in the full range.
///
Double_t TRestRawSignal::GetIntegral() {
    if (fRange.X() < 0) fRange.SetX(0);
    if (fRange.Y() <= 0 || fRange.Y() > GetNumberOfPoints()) fRange.SetY(GetNumberOfPoints());

    Double_t sum = 0;
    for (int i = fRange.X(); i < fRange.Y(); i++) sum += GetData(i);
    return sum;
}

///////////////////////////////////////////////
/// \brief It returns the integral of points found in the specific range given
/// by (startBin,endBin).
///
Double_t TRestRawSignal::GetIntegralInRange(Int_t startBin, Int_t endBin) {
    if (startBin < 0) {
        startBin = 0;
    }
    if (endBin <= 0 || endBin > Int_t(GetNumberOfPoints())) {
        endBin = GetNumberOfPoints();
    }

    Double_t sum = 0;
    for (int i = startBin; i < endBin; i++) sum += GetRawData(i);
    return sum;
}

///////////////////////////////////////////////
/// \brief It returns the integral of points identified over threshold.
/// InitializePointsOverThreshold should
/// have been called first.
///
Double_t TRestRawSignal::GetThresholdIntegral() {
    if (fThresholdIntegral == -1)
        if (fShowWarnings) {
            std::cout << "TRestRawSignal::GetThresholdIntegral. "
                         "InitializePointsOverThreshold should be "
                         "called first!"
                      << endl;
            fShowWarnings = false;
        }
    return fThresholdIntegral;
}

///////////////////////////////////////////////
/// \brief It returns the integral of points identified over threshold found in
/// the first positive rise of the
/// signal. InitializePointsOverThreshold should have been called first.
///
Double_t TRestRawSignal::GetSlopeIntegral() {
    if (fThresholdIntegral == -1)
        cout << "REST Warning. TRestRawSignal::GetSlopeIntegral. "
                "InitializePointsOverThreshold should be called first."
             << endl;

    Double_t sum = 0;
    for (unsigned int n = 0; n < fPointsOverThreshold.size(); n++) {
        if (n + 1 >= fPointsOverThreshold.size()) return sum;

        sum += GetData(fPointsOverThreshold[n]);

        if (GetData(fPointsOverThreshold[n + 1]) - GetData(fPointsOverThreshold[n]) < 0) break;
    }
    return sum;
}

///////////////////////////////////////////////
/// \brief It returns the rate of change or slope from the points that have been
/// identified over threshlold on
/// the first positive rise of the signal. InitializePointsOverThreshold should
/// have been called first.
///
Double_t TRestRawSignal::GetRiseSlope() {
    if (fThresholdIntegral == -1)
        cout << "REST Warning. TRestRawSignal::GetRiseSlope. "
                "InitializePointsOverThreshold should be called first."
             << endl;

    if (fPointsOverThreshold.size() < 2) {
        // cout << "REST Warning. TRestRawSignal::GetRiseSlope. Less than 2 points!." << endl;
        return 0;
    }

    Int_t maxBin = GetMaxPeakBin() - 1;

    Double_t hP = GetData(maxBin);

    Double_t lP = GetData(fPointsOverThreshold[0]);

    return (hP - lP) / (maxBin - fPointsOverThreshold[0]);
}

///////////////////////////////////////////////
/// \brief It returns the time required from the signal to reach the maximum.
/// InitializePointsOverThreshold should have been called first.
///
Int_t TRestRawSignal::GetRiseTime() {
    if (fThresholdIntegral == -1)
        cout << "REST Warning. TRestRawSignal::GetRiseTime. "
                "InitializePointsOverThreshold should be called first."
             << endl;

    if (fPointsOverThreshold.size() < 2) {
        // cout << "REST Warning. TRestRawSignal::GetRiseTime. Less than 2 points!." << endl;
        return 0;
    }

    return GetMaxPeakBin() - fPointsOverThreshold[0];
}

///////////////////////////////////////////////
/// \brief It returns the integral calculated using the maximum signal amplitude
/// and its neightbour points.
///
Double_t TRestRawSignal::GetTripleMaxIntegral() {
    if (fRange.X() < 0) fRange.SetX(0);
    if (fRange.Y() <= 0 || fRange.Y() > GetNumberOfPoints()) fRange.SetY(GetNumberOfPoints());

    if (fThresholdIntegral == -1) {
        cout << "REST Warning. TRestRawSignal::GetTripleMaxIntegral. "
                "InitializePointsOverThreshold should be called first."
             << endl;
        return 0;
    }

    if (fPointsOverThreshold.size() < 2) {
        // cout << "REST Warning. TRestRawSignal::GetTripleMaxIntegral. Points over
        // "
        //        "threshold = "
        //     << fPointsOverThreshold.size() << endl;
        return 0;
    }

    Int_t cBin = GetMaxPeakBin();

    if (cBin + 1 >= Int_t(GetNumberOfPoints())) {
        return 0;
    }

    Double_t a1 = GetData(cBin);
    Double_t a2 = GetData(cBin - 1);
    Double_t a3 = GetData(cBin + 1);

    return a1 + a2 + a3;
}

///////////////////////////////////////////////
/// \brief It returns the average of the points found in the range (startBin,
/// endBin)
///
Double_t TRestRawSignal::GetAverageInRange(Int_t startBin, Int_t endBin) {
    if (startBin < 0) startBin = 0;
    if (endBin <= 0 || endBin > Int_t(GetNumberOfPoints())) {
        endBin = GetNumberOfPoints();
    }

    Double_t sum = 0;
    for (int i = startBin; i <= endBin; i++) sum += this->GetData(i);

    return sum / (endBin - startBin + 1);
}

///////////////////////////////////////////////
/// \brief It returns the temporal width of the peak with maximum amplitude
/// inside the signal
///
Int_t TRestRawSignal::GetMaxPeakWidth() {
    Int_t mIndex = this->GetMaxPeakBin();
    Double_t maxValue = this->GetData(mIndex);

    Double_t value = maxValue;
    Int_t rightIndex = mIndex;
    while (value > maxValue / 2) {
        value = this->GetData(rightIndex);
        rightIndex++;
    }
    Int_t leftIndex = mIndex;
    value = maxValue;
    while (value > maxValue / 2) {
        value = this->GetData(leftIndex);
        leftIndex--;
    }

    return rightIndex - leftIndex;
}

///////////////////////////////////////////////
/// \brief It returns the amplitude of the signal maximum, baseline will be
/// corrected if CalculateBaseLine was
/// called first.
///
Double_t TRestRawSignal::GetMaxPeakValue() { return GetData(GetMaxPeakBin()); }

///////////////////////////////////////////////
/// \brief It returns the bin at which the maximum peak amplitude happens
///
Int_t TRestRawSignal::GetMaxPeakBin() {
    Double_t max = numeric_limits<Double_t>::min();

    Int_t index = 0;

    if (fRange.Y() == 0 || fRange.Y() > GetNumberOfPoints()) fRange.SetY(GetNumberOfPoints());
    if (fRange.X() < 0) fRange.SetX(0);

    for (int i = fRange.X(); i < fRange.Y(); i++) {
        if (this->GetData(i) > max) {
            max = GetData(i);
            index = i;
        }
    }

    return index;
}

///////////////////////////////////////////////
/// \brief It returns the amplitude of the signal minimum, baseline will be
/// corrected if CalculateBaseLine was
/// called first.
///
Double_t TRestRawSignal::GetMinPeakValue() { return GetData(GetMinPeakBin()); }

///////////////////////////////////////////////
/// \brief It returns the bin at which the minimum peak amplitude happens
///
Int_t TRestRawSignal::GetMinPeakBin() {
    Double_t min = numeric_limits<Double_t>::max();
    Int_t index = 0;

    if (fRange.Y() == 0 || fRange.Y() > GetNumberOfPoints()) fRange.SetY(GetNumberOfPoints());
    if (fRange.X() < 0) fRange.SetX(0);

    for (int i = fRange.X(); i < fRange.Y(); i++) {
        if (this->GetData(i) < min) {
            min = GetData(i);
            index = i;
        }
    }

    return index;
}

///////////////////////////////////////////////
/// \brief It returns whether the signal has ADC saturation
///
Bool_t TRestRawSignal::IsADCSaturation(int Nflat) {
    if (Nflat <= 0) return false;
    // GetMaxPeakBin() will always find the first max peak bin if multiple bins are in same max value.
    int index = GetMaxPeakBin();
    Short_t value = fSignalData[index];

    bool sat = false;
    if (index + Nflat <= (int)fSignalData.size()) {
        for (int i = index; i < index + Nflat; i++) {
            if (fSignalData[i] != value) {
                break;
            }
            if (i == index + Nflat - 1) {
                sat = true;
            }
        }
    }

    return sat;
}

///////////////////////////////////////////////
/// \brief It calculates the differential signal of the existing signal and it
/// will place at the
/// signal pointer given by argument.
///
/// \param smearPoints is a number bigger that 0 that serves to change the time
/// distance of points used to
/// obtain the differential at a given point.
///
void TRestRawSignal::GetDifferentialSignal(TRestRawSignal* diffSignal, Int_t smearPoints) {
    if (smearPoints <= 0) smearPoints = 1;
    diffSignal->Initialize();

    for (int i = 0; i < smearPoints; i++) {
        diffSignal->AddPoint((Short_t)0);
    }

    for (int i = smearPoints; i < Int_t(this->GetNumberOfPoints()) - smearPoints; i++) {
        Double_t value = 0.5 * (this->GetData(i + smearPoints) - GetData(i - smearPoints)) / smearPoints;

        diffSignal->AddPoint((Short_t)value);
    }

    for (int i = GetNumberOfPoints() - smearPoints; i < int(GetNumberOfPoints()); i++) {
        diffSignal->AddPoint((Short_t)0);
    }
}

///////////////////////////////////////////////
/// \brief It calculates an arbitrary Gaussian noise placing it at the signal
/// pointer given by argument. The
/// number of points defined will be the same as the existing signal.
///
/// \param noiseLevel It defines the amplitude of the signal noise fluctuations
/// as its standard deviation.
///
void TRestRawSignal::GetWhiteNoiseSignal(TRestRawSignal* noiseSignal, Double_t noiseLevel) {
    TRandom3 random(fSeed);

    for (int i = 0; i < int(GetNumberOfPoints()); i++) {
        Double_t value = this->GetData(i) + random.Gaus(0, noiseLevel);
        // do not cast as short so that there are no problems with overflows
        // (https://github.com/rest-for-physics/rawlib/issues/113)
        noiseSignal->AddPoint(value);
    }
}

///////////////////////////////////////////////
/// \brief It smoothes the existing signal and places it at the signal pointer
/// given by argument.
///
/// \param averagingPoints It defines the number of neighbour consecutive
/// points used to average the signal
///
void TRestRawSignal::GetSignalSmoothed(TRestRawSignal* smoothedSignal, Int_t averagingPoints) {
    smoothedSignal->Initialize();

    averagingPoints = (averagingPoints / 2) * 2 + 1;  // make it odd >= averagingPoints

    Double_t sumAvg = GetIntegralInRange(0, averagingPoints) / averagingPoints;

    for (int i = 0; i <= averagingPoints / 2; i++) smoothedSignal->AddPoint((Short_t)sumAvg);

    for (int i = averagingPoints / 2 + 1; i < int(GetNumberOfPoints()) - averagingPoints / 2; i++) {
        sumAvg -= this->GetRawData(i - (averagingPoints / 2 + 1)) / averagingPoints;
        sumAvg += this->GetRawData(i + averagingPoints / 2) / averagingPoints;
        smoothedSignal->AddPoint((Short_t)sumAvg);
    }

    for (int i = GetNumberOfPoints() - averagingPoints / 2; i < int(GetNumberOfPoints()); i++)
        smoothedSignal->AddPoint(sumAvg);
}

///////////////////////////////////////////////
/// \brief It smoothes the existing signal and returns it in a vector of Float_t values
///
/// \param averagingPoints It defines the number of neighbour consecutive
/// points used to average the signal
///
/// \param option If the option is set to "EXCLUDE OUTLIERS", points that are too far away from the median
/// baseline will be ignored to improve the smoothing result
///
std::vector<Float_t> TRestRawSignal::GetSignalSmoothed(Int_t averagingPoints, std::string option) {
    std::vector<Float_t> result;

    if (option == "") {
        result.resize(GetNumberOfPoints());

        averagingPoints = (averagingPoints / 2) * 2 + 1;  // make it odd >= averagingPoints

        Float_t sumAvg = (Float_t)GetIntegralInRange(0, averagingPoints) / averagingPoints;

        for (int i = 0; i <= averagingPoints / 2; i++) result[i] = sumAvg;

        for (int i = averagingPoints / 2 + 1; i < int(GetNumberOfPoints()) - averagingPoints / 2; i++) {
            sumAvg -= this->GetRawData(i - (averagingPoints / 2 + 1)) / averagingPoints;
            sumAvg += this->GetRawData(i + averagingPoints / 2) / averagingPoints;
            result[i] = sumAvg;
        }

        for (int i = GetNumberOfPoints() - averagingPoints / 2; i < int(GetNumberOfPoints()); i++)
            result[i] = sumAvg;
    } else if (ToUpper(option) == "EXCLUDE OUTLIERS") {
        result = GetSignalSmoothed_ExcludeOutliers(averagingPoints);
    } else {
        cout << "TRestRawSignal::GetSignalSmoothed. Error! No such option!" << endl;
    }
    return result;
}

///////////////////////////////////////////////
/// \brief It smooths the existing signal and returns it in a vector of Float_t values. This method excludes
/// points which are far off from the BaseLine IQR (e.g. signals). In case the baseline parameters were not
/// calculated yet, this method calls CalculateBaseLine with the "ROBUST" option on the entire signal range
/// minus 5 bins on the edges.
///
/// \param averagingPoints It defines the number of neightbour consecutive
/// points used to average the signal
///
std::vector<Float_t> TRestRawSignal::GetSignalSmoothed_ExcludeOutliers(Int_t averagingPoints) {
    std::vector<Float_t> result(GetNumberOfPoints());

    if (fBaseLine == 0) CalculateBaseLine(5, GetNumberOfPoints() - 5, "ROBUST");

    averagingPoints = (averagingPoints / 2) * 2 + 1;  // make it odd >= averagingPoints

    Float_t sumAvg = (Float_t)GetIntegralInRange(0, averagingPoints) / averagingPoints;

    // Points at the beginning, where we can calculate a moving average
    for (int i = 0; i <= averagingPoints / 2; i++) result[i] = sumAvg;

    // Points in the middle
    float_t amplitude;
    for (int i = averagingPoints / 2 + 1; i < int(GetNumberOfPoints()) - averagingPoints / 2; i++) {
        amplitude = this->GetRawData(i - (averagingPoints / 2 + 1));
        sumAvg -= (std::abs(amplitude - fBaseLine) > 3 * fBaseLineSigma) ? fBaseLine / averagingPoints
                                                                         : amplitude / averagingPoints;
        amplitude = this->GetRawData(i + averagingPoints / 2);
        sumAvg += (std::abs(amplitude - fBaseLine) > 3 * fBaseLineSigma) ? fBaseLine / averagingPoints
                                                                         : amplitude / averagingPoints;
        result[i] = sumAvg;
    }

    // Points at the end, where we can calculate a moving average
    for (int i = GetNumberOfPoints() - averagingPoints / 2; i < int(GetNumberOfPoints()); i++)
        result[i] = sumAvg;
    return result;
}

///////////////////////////////////////////////
/// \brief It applies the moving average filter (GetSignalSmoothed) to the signal, which is then subtracted
/// from the raw data, resulting in a corrected baseline. The returned signal is placed at the signal pointer
/// given by the argument.
///
/// \param smoothedSignal The pointer to the TRestRawSignal which will contain the corrected signal
///
/// \param averagingPoints It defines the number of neighbour consecutive
/// points used to average the signal
///
void TRestRawSignal::GetBaseLineCorrected(TRestRawSignal* smoothedSignal, Int_t averagingPoints) {
    smoothedSignal->Initialize();

    std::vector<Float_t> averagedSignal = GetSignalSmoothed(averagingPoints, "EXCLUDE OUTLIERS");

    for (int i = 0; i < int(GetNumberOfPoints()); i++) {
        smoothedSignal->AddPoint(GetRawData(i) - averagedSignal[i]);
    }
}

///////////////////////////////////////////////
/// \brief This method is called by CalculateBaseLine and is used to determine the value of the baseline as
/// average (arithmetic mean) of the data points found
/// in the range defined between startBin and endBin.
///
void TRestRawSignal::CalculateBaseLineMean(Int_t startBin, Int_t endBin) {
    if (endBin - startBin <= 0) {
        fBaseLine = 0.;
    } else if (endBin > (int)fSignalData.size()) {
        cout << "TRestRawSignal::CalculateBaseLine. Error! Baseline range exceeds the rawdata depth!!"
             << endl;
        endBin = fSignalData.size();
    } else {
        Double_t baseLine = 0;
        for (int i = startBin; i < endBin; i++) baseLine += fSignalData[i];
        fBaseLine = baseLine / (endBin - startBin);
    }
}

///////////////////////////////////////////////
/// \brief This method is called by CalculateBaseLine with the "ROBUST"-option and is used to determine the
/// value of the baseline as the median of the data points found in the range defined between startBin and
/// endBin.
///
void TRestRawSignal::CalculateBaseLineMedian(Int_t startBin, Int_t endBin) {
    if (endBin - startBin <= 0) {
        fBaseLine = 0.;
    } else if (endBin > (int)fSignalData.size()) {
        cout << "TRestRawSignal::CalculateBaseLine. Error! Baseline range exceeds the rawdata depth!!"
             << endl;
        endBin = fSignalData.size();
    } else {
        vector<Short_t>::const_iterator first = fSignalData.begin() + startBin;
        vector<Short_t>::const_iterator last = fSignalData.begin() + endBin;
        vector<Short_t> v(first, last);
        const Short_t* signalInRange = &v[0];
        fBaseLine = TMath::Median(endBin - startBin, signalInRange);
    }
}

///////////////////////////////////////////////
/// \brief This method calculates the average and fluctuation of the baseline in the
/// specified range and writes the values to fBaseLine and fBaseLineSigma respectively.
/// Without further option, this method calculates the average as arithmetic mean,
/// and the fluctuation as standard deviation.
///
/// \param option By setting this option to "ROBUST", the average is calculated as median,
/// and the fluctuation as interquartile range (IQR), which are less affected by outliers (e.g. a signal
/// pulse).
///
void TRestRawSignal::CalculateBaseLine(Int_t startBin, Int_t endBin, const std::string& option) {
    if (ToUpper(option) == "ROBUST") {
        CalculateBaseLineMedian(startBin, endBin);
        CalculateBaseLineSigmaIQR(startBin, endBin);
    } else {
        CalculateBaseLineMean(startBin, endBin);
        CalculateBaseLineSigmaSD(startBin, endBin);
    }
}

///////////////////////////////////////////////
/// \brief This method is called by CalculateBaseLine to
/// determine the value of the baseline
/// fluctuation as its standard deviation in the baseline range provided.
///
void TRestRawSignal::CalculateBaseLineSigmaSD(Int_t startBin, Int_t endBin) {
    if (endBin - startBin <= 0) {
        fBaseLineSigma = 0;
    } else {
        Double_t baseLineSigma = 0;
        for (int i = startBin; i < endBin; i++)
            baseLineSigma += (fBaseLine - fSignalData[i]) * (fBaseLine - fSignalData[i]);
        fBaseLineSigma = TMath::Sqrt(baseLineSigma / (endBin - startBin));
    }
}

///////////////////////////////////////////////
/// \brief This method is called by CalculateBaseLine with the "ROBUST"-option to
/// determine the value of the baseline
/// fluctuation as its interquartile range (IQR) in the baseline range provided. The IQR is more robust
/// towards outliers than the standard deviation.
///
void TRestRawSignal::CalculateBaseLineSigmaIQR(Int_t startBin, Int_t endBin) {
    if (endBin - startBin <= 0) {
        fBaseLineSigma = 0;
    } else {
        vector<Short_t>::const_iterator first = fSignalData.begin() + startBin;
        vector<Short_t>::const_iterator last = fSignalData.begin() + endBin;
        vector<Short_t> v(first, last);
        std::sort(v.begin(), v.end());
        Short_t Q1 = v[(int)(endBin - startBin) * 0.25];
        Short_t Q3 = v[(int)(endBin - startBin) * 0.75];
        Double_t IQR = Q3 - Q1;
        fBaseLineSigma =
            IQR / 1.349;  // IQR/1.349 equals the standard deviation in case of normally distributed data
    }
}

///////////////////////////////////////////////
/// \brief This method adds an offset to the signal data
///
void TRestRawSignal::AddOffset(Short_t offset) {
    if (fBaseLine != 0 || fBaseLineSigma != 0) fBaseLineSigma += (Double_t)offset;
    for (int i = 0; i < int(GetNumberOfPoints()); i++) fSignalData[i] = fSignalData[i] + offset;
}

///////////////////////////////////////////////
/// \brief This method scales the signal by a given value
///
void TRestRawSignal::Scale(Double_t value) {
    for (int i = 0; i < int(GetNumberOfPoints()); i++) {
        Double_t scaledValue = value * fSignalData[i];
        fSignalData[i] = (Short_t)scaledValue;
    }
}

///////////////////////////////////////////////
/// \brief This method adds the signal provided by argument to the existing
/// signal.
///
void TRestRawSignal::SignalAddition(const TRestRawSignal& signal) {
    if (this->GetNumberOfPoints() != signal.GetNumberOfPoints()) {
        cout << "ERROR : TRestRawSignal::SignalAddition." << endl;
        cout << "I cannot add two signals with different number of points" << endl;
        return;
    }

    for (int i = 0; i < int(GetNumberOfPoints()); i++) {
        fSignalData[i] += signal.GetData(i);
    }
}

///////////////////////////////////////////////
/// \brief This method dumps to a text file the data inside fSignalData.
///
void TRestRawSignal::WriteSignalToTextFile(const TString& filename) {
    // We should check it is writable
    FILE* file = fopen(filename.Data(), "w");
    for (int i = 0; i < int(GetNumberOfPoints()); i++) fprintf(file, "%d\t%lf\n", i, GetData(i));
    fclose(file);
}

///////////////////////////////////////////////
/// \brief It prints the signal data on screen.
///
void TRestRawSignal::Print() const {
    cout << "---------------------" << endl;
    cout << "Signal id : " << this->GetSignalID() << endl;
    cout << "Baseline : " << fBaseLine << endl;
    cout << "Baseline sigma : " << fBaseLineSigma << endl;
    cout << "+++++++++++++++++++++" << endl;
    for (int i = 0; i < int(GetNumberOfPoints()); i++)
        cout << "Bin : " << i << " amplitude : " << GetRawData(i) << endl;
    cout << "---------------------" << endl;
}

///////////////////////////////////////////////
/// \brief It builds a TGraph object that can be used for drawing.
///
TGraph* TRestRawSignal::GetGraph(Int_t color) {
    delete fGraph;
    fGraph = new TGraph();

    fGraph->SetLineWidth(2);
    fGraph->SetLineColor(color % 8 + 1);
    fGraph->SetMarkerStyle(7);

    for (int i = 0; i < int(GetNumberOfPoints()); i++) {
        fGraph->SetPoint(i, i, GetData(i));
    }

    fGraph->GetXaxis()->SetLimits(0, GetNumberOfPoints() - 1);

    /*
     * To draw x axis in multiples of 2
    for (int i = 0; i < values.size(); i++) {
        if (i % 32 != 0 && i != values.size() - 1){
            continue;
        }
        fGraph->GetXaxis()->SetBinLabel(fGraph->GetXaxis()->FindBin(i), std::to_string(i).c_str());
    }
     */

    return fGraph;
}

vector<pair<UShort_t, double>> TRestRawSignal::GetPeaks(double threshold, double distance) const {
    vector<pair<UShort_t, double>> peaks;

    for (UShort_t i = 0; i < GetNumberOfPoints(); i++) {
        const auto point = GetData(i);
        if (i > 0 && i < GetNumberOfPoints() - 1) {
            double prevPoint = GetData(i - 1);
            double nextPoint = GetData(i + 1);

            if (point >= threshold && point >= prevPoint && point >= nextPoint) {
                // Check if the peak is spaced far enough from the previous peak
                if (peaks.empty() || i - peaks.back().first >= distance) {
                    peaks.push_back(std::make_pair(i, point));
                }
            }
        }
    }
    return peaks;
}
