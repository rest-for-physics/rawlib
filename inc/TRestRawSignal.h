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

#ifndef RestCore_TRestRawSignal
#define RestCore_TRestRawSignal

#include <TGraph.h>
#include <TRandom.h>
#include <TString.h>
#include <TVector2.h>

#include <iostream>
#include <string>
#include <vector>

//! It defines a Short_t array with a physical parameter that evolves in time using a fixed time bin.
class TRestRawSignal {
   private:
    void CalculateThresholdIntegral();

    void CalculateBaseLineSigmaSD(Int_t startBin, Int_t endBin);

    void CalculateBaseLineSigmaIQR(Int_t startBin, Int_t endBin);

    void CalculateBaseLineSigmaExcludeOutliers(Int_t startBin, Int_t endBin);

    std::vector<Float_t> GetSignalSmoothed_ExcludeOutliers(Int_t averagingPoints);

   protected:
    /// An integer value used to attribute a unique identification number to the signal.
    Int_t fSignalID;

    /// Vector with the data of the signal
    std::vector<Short_t> fSignalData;

    Bool_t fShowWarnings = true;

    /// Seed used for random number generation
    UInt_t fSeed = gRandom->GetSeed();

   public:
    /// A TGraph pointer used to store the TRestRawSignal drawing
    TGraph* fGraph;  //!

    /// A std::vector containing the index of points that are identified over threshold.
    std::vector<Int_t> fPointsOverThreshold;  //!

    /// It stores the integral value obtained from the points identified over threshold.
    Double_t fThresholdIntegral = -1;  //!

    /// It defines the number of points to include before point over threshold definition. NOT implemented.
    Int_t fHeadPoints;  //!

    /// It defines the number of points to include after point over threshold definition. NOT implemented.
    Int_t fTailPoints;  //!

    /// This baseline value will be subtracted from GetData for any raw signal observable calculation.
    Double_t fBaseLine = 0;  //!

    /// The baseline fluctuation calculated as the standard deviation of the baseline.
    Double_t fBaseLineSigma = 0;  //!

    /// Any signal calculation will be restricted to the following range definition.
    TVector2 fRange = TVector2(0, 0);  //!

    /// Returns the value of signal ID
    inline Int_t GetSignalID() const { return fSignalID; }

    /// Returns the value of signal ID
    inline Int_t GetID() const { return fSignalID; }

    /// Returns the actual number of points, or size of the signal
    inline Int_t GetNumberOfPoints() const { return fSignalData.size(); }

    /// Returns a std::vector containing the indexes of data points over threshold
    inline std::vector<Int_t> GetPointsOverThreshold() const { return fPointsOverThreshold; }

    /// Returns the maximum value found in the data points. It includes baseline correction
    inline Double_t GetMaxValue() { return GetMaxPeakValue(); }

    /// Returns the lowest value found in the data points. It includes baseline correction
    inline Double_t GetMinValue() { return GetMinPeakValue(); }

    /// Returns the number of head points used on points over threshold definition
    inline Int_t GetHeadPoints() const { return fHeadPoints; }

    /// Returns the number of tail points used on points over threshold definition
    inline Int_t GetTailPoints() const { return fTailPoints; }

    /// Returns the value of baseline that it is initialized after calling
    /// CalculateBaseLine.
    inline Double_t GetBaseLine() const { return fBaseLine; }

    /// Returns the value of baseline sigma that it is initialized after calling CalculateBaseLineSigmaSD or
    /// CalculateBaseLineSigmaIQR.
    inline Double_t GetBaseLineSigma() const { return fBaseLineSigma; }

    /// Returns the range defined by user
    inline TVector2 GetRange() const { return fRange; }

    /// Returns false if the baseline and its baseline fluctuation was not initialized.
    inline Bool_t isBaseLineInitialized() const { return !(fBaseLineSigma == 0 && fBaseLine == 0); }

    Double_t GetData(Int_t n) const;

    Double_t GetRawData(Int_t n) const;

    Short_t operator[](Int_t n);

    /// It sets the id number of the signal
    inline void SetSignalID(Int_t sID) { fSignalID = sID; }

    /// It sets the id number of the signal
    inline void SetID(Int_t sID) { fSignalID = sID; }

    /// It sets the number of head points
    inline void SetHeadPoints(Int_t p) { fHeadPoints = p; }

    /// It sets the number of tail points
    inline void SetTailPoints(Int_t p) { fTailPoints = p; }

    /// It sets/constrains the range for any calculation.
    inline void SetRange(const TVector2& range) { fRange = range; }

    inline void SetRangeToMax() { fRange = TVector2(0, GetNumberOfPoints()); }

    /// It sets/constrains the range for any calculation.
    inline void SetRange(Int_t from, Int_t to) { fRange = TVector2(from, to); }

    void Reset();

    void Initialize();

    void AddPoint(Short_t);

    void AddPoint(Double_t);

    void IncreaseBinBy(Int_t bin, Double_t data);

    void InitializePointsOverThreshold(const TVector2& thrPar, Int_t nPointsOver, Int_t nPointsFlat = 512);

    UInt_t GetSeed() const { return fSeed; }

    Double_t GetIntegral();

    Double_t GetIntegralInRange(Int_t startBin, Int_t endBin);

    Double_t GetThresholdIntegral();

    Double_t GetTripleMaxIntegral();

    Double_t GetSlopeIntegral();

    Double_t GetRiseSlope();

    Int_t GetRiseTime();

    Double_t GetAverageInRange(Int_t startBin, Int_t endBin);

    Int_t GetMaxPeakWidth();

    Double_t GetMaxPeakValue();

    Int_t GetMaxPeakBin();

    Double_t GetMinPeakValue();

    Int_t GetMinPeakBin();

    Bool_t IsADCSaturation(int Nflat = 3);

    void GetDifferentialSignal(TRestRawSignal* diffSignal, Int_t smearPoints);

    void GetSignalSmoothed(TRestRawSignal* smoothedSignal, Int_t averagingPoints);

    std::vector<Float_t> GetSignalSmoothed(Int_t averagingPoints, std::string option = "");

    void GetWhiteNoiseSignal(TRestRawSignal* noiseSignal, Double_t noiseLevel = 1.);

    void CalculateBaseLineMean(Int_t startBin, Int_t endBin);

    void CalculateBaseLineMedian(Int_t startBin, Int_t endBin);

    void CalculateBaseLineMedianExcludeOutliers(Int_t startBin, Int_t endBin);

    void CalculateBaseLine(Int_t startBin, Int_t endBin, const std::string& option = "");

    void GetBaseLineCorrected(TRestRawSignal* smoothedSignal, Int_t averagingPoints);

    void AddOffset(Short_t offset);

    void SignalAddition(const TRestRawSignal& signal);

    void Scale(Double_t value);

    void WriteSignalToTextFile(const TString& filename);

    void Print() const;

    void SetSeed(UInt_t seed) { fSeed = seed; }

    TGraph* GetGraph(Int_t color = 1);

    /// Returns the (time, amplitude) of the peaks in the signal.
    /// Peaks are defined as the points that are above the threshold and are separated by a minimum distance
    /// in time bin units. The threshold must be set in absolute value (regardless of the baseline)
    std::vector<std::pair<UShort_t, double>> GetPeaks(double threshold, UShort_t distance = 5) const;
    std::vector<std::pair<UShort_t, double>> GetPeaksVeto(double threshold, UShort_t distance = 5) const;

    TRestRawSignal();
    TRestRawSignal(Int_t nBins);
    ~TRestRawSignal();

    ClassDef(TRestRawSignal, 2);
};
#endif
