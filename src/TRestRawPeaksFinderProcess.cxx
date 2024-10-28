
#include "TRestRawPeaksFinderProcess.h"

#include <utility>

ClassImp(TRestRawPeaksFinderProcess);

using namespace std;

void TRestRawPeaksFinderProcess::InitProcess() {}

TRestEvent* TRestRawPeaksFinderProcess::ProcessEvent(TRestEvent* inputEvent) {
    fInputEvent = dynamic_cast<TRestRawSignalEvent*>(inputEvent);

    const auto run = GetRunInfo();
    if (run != nullptr) {
        fInputEvent->InitializeReferences(run);
    }

    if (fReadoutMetadata == nullptr) {
        fReadoutMetadata = fInputEvent->GetReadoutMetadata();
    }

    if (fReadoutMetadata == nullptr && !fChannelTypes.empty()) {
        cerr << "TRestRawPeaksFinderProcess::ProcessEvent: readout metadata is null, cannot filter "
                "the process by signal type"
             << endl;
        exit(1);
    }

    vector<tuple<UShort_t, UShort_t, double>> eventPeaks;  // signalId, time, amplitude

    for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
        const auto signal = fInputEvent->GetSignal(signalIndex);
        const UShort_t signalId = signal->GetSignalID();

        const string channelType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);
        const string channelName = fReadoutMetadata->GetNameForChannelDaqId(signalId);

        // check if channel type is in the list of selected channel types
        if (fChannelTypes.find(channelType) == fChannelTypes.end()) {
            continue;
        }

        // Choose appropriate function based on channel type
        if (channelType == "tpc") {
            signal->CalculateBaseLine(fBaselineRange.X(), fBaselineRange.Y());
            const auto peaks =
                signal->GetPeaks(signal->GetBaseLine() + 5 * signal->GetBaseLineSigma(), fDistance);

            for (const auto& [time, amplitude] : peaks) {
                eventPeaks.emplace_back(signalId, time, amplitude);
            }
        } else if (channelType == "veto") {
            // For veto signals the baseline is calculated over the whole range, as we don´t know where the
            // signal will be.
            signal->CalculateBaseLine(0, 511, "robust");
            // For veto signals the threshold is selected by the user.
            const auto peaks =
                signal->GetPeaksVeto(signal->GetBaseLine() + fThresholdOverBaseline, fDistance);

            for (const auto& [time, amplitude] : peaks) {
                eventPeaks.emplace_back(signalId, time, amplitude);
            }
        }
    }

    // sort eventPeaks by time, then signal id
    sort(eventPeaks.begin(), eventPeaks.end(),
         [](const tuple<UShort_t, UShort_t, double>& a, const tuple<UShort_t, UShort_t, double>& b) {
             return tie(get<1>(a), get<0>(a)) < tie(get<1>(b), get<0>(b));
         });

    vector<UShort_t> peaksChannelId;
    vector<UShort_t> peaksTime;
    vector<double> peaksAmplitude;

    double peaksEnergy = 0.0;
    UShort_t peaksCount = 0;
    UShort_t peaksCountUnique = 0;

    set<UShort_t> uniquePeaks;
    for (const auto& [channelId, time, amplitude] : eventPeaks) {
        peaksChannelId.push_back(channelId);
        peaksTime.push_back(time);
        peaksAmplitude.push_back(amplitude);

        peaksEnergy += amplitude;
        peaksCount++;

        if (uniquePeaks.find(channelId) == uniquePeaks.end()) {
            uniquePeaks.insert(channelId);
            peaksCountUnique++;
        }
    }

    SetObservableValue("peaksChannelId", peaksChannelId);
    SetObservableValue("peaksTimeBin", peaksTime);
    SetObservableValue("peaksAmplitudeADC", peaksAmplitude);

    SetObservableValue("peaksAmplitudeADCSum", peaksEnergy);
    SetObservableValue("peaksCount", peaksCount);
    SetObservableValue("peaksCountUnique", peaksCountUnique);

    vector<UShort_t> windowPeakIndex(eventPeaks.size(), std::numeric_limits<UShort_t>::max());
    vector<UShort_t> windowPeakCenter(eventPeaks.size(), 0);
    vector<UShort_t> windowPeakMultiplicity(eventPeaks.size(), 0);

    UShort_t window_index = 0;
    for (size_t peakIndex = 0; peakIndex < eventPeaks.size(); peakIndex++) {
        const auto& [channelId, time, amplitude] = eventPeaks[peakIndex];
        const auto windowTimeStart = time - fWindow / 2;
        const auto windowTimeEnd = time + fWindow / 2;

        // check if the peak is already in a window
        if (windowPeakIndex[peakIndex] != std::numeric_limits<UShort_t>::max()) {
            continue;
        }

        UShort_t window_center_time = time;

        set<UShort_t> windowPeaksIndex;

        // add the peak to the window
        windowPeaksIndex.insert(peakIndex);

        // add the peaks that are in the window
        for (size_t otherPeakIndex = peakIndex + 1; otherPeakIndex < eventPeaks.size(); otherPeakIndex++) {
            const auto& [otherChannelId, otherTime, otherAmplitude] = eventPeaks[otherPeakIndex];

            if (otherTime < windowTimeStart) {
                continue;
            }

            if (otherTime > windowTimeEnd) {
                break;
            }

            windowPeaksIndex.insert(otherPeakIndex);
        }

        for (const auto& index : windowPeaksIndex) {
            windowPeakIndex[index] = window_index;
            windowPeakCenter[index] = window_center_time;
            windowPeakMultiplicity[index] = windowPeaksIndex.size();
        }

        window_index++;
    }

    SetObservableValue("windowPeakIndex", windowPeakIndex);
    SetObservableValue("windowPeakTimeBin", windowPeakCenter);
    SetObservableValue("windowPeakMultiplicity", windowPeakMultiplicity);

    vector<UShort_t> windowCenter;
    vector<UShort_t> windowMultiplicity;

    set<UShort_t> windowPeaksIndexInserted;
    for (size_t peakIndex = 0; peakIndex < eventPeaks.size(); peakIndex++) {
        const auto& window_peak_index = windowPeakIndex[peakIndex];
        if (windowPeaksIndexInserted.find(window_peak_index) != windowPeaksIndexInserted.end()) {
            continue;
        }

        windowPeaksIndexInserted.insert(window_peak_index);

        windowCenter.push_back(windowPeakCenter[peakIndex]);
        windowMultiplicity.push_back(windowPeakMultiplicity[peakIndex]);
    }

    SetObservableValue("windowTimeBin", windowCenter);
    SetObservableValue("windowMultiplicity", windowMultiplicity);

    if (fTimeBinToTimeFactorMultiplier != 0.0) {
        vector<Double_t> windowCenterTime(windowCenter.size(), 0.0);
        vector<Double_t> windowPeakCenterTime(windowPeakCenter.size(), 0.0);
        vector<Double_t> peaksTimePhysical(peaksTime.size(), 0.0);

        // lambda to convert time bin to time
        auto timeBinToTime = [this](UShort_t timeBin) {
            return fTimeBinToTimeFactorMultiplier * timeBin - fTimeBinToTimeFactorOffset;
        };

        for (size_t i = 0; i < windowCenter.size(); i++) {
            windowCenterTime[i] = timeBinToTime(windowCenter[i]);
        }

        for (size_t i = 0; i < windowPeakCenter.size(); i++) {
            windowPeakCenterTime[i] = timeBinToTime(windowPeakCenter[i]);
        }

        for (size_t i = 0; i < peaksTime.size(); i++) {
            peaksTimePhysical[i] = timeBinToTime(peaksTime[i]);
        }

        SetObservableValue("windowTime", windowCenterTime);
        SetObservableValue("windowPeakTime", windowPeakCenterTime);
        SetObservableValue("peaksTime", peaksTimePhysical);
    }

    if (fADCtoEnergyFactor != 0.0 || !fChannelIDToADCtoEnergyFactor.empty()) {
        vector<Double_t> peaksEnergyPhysical(peaksAmplitude.size(), 0.0);
        Double_t peaksEnergySum = 0.0;

        if (fADCtoEnergyFactor != 0.0) {
            // same factor for all peaks
            for (size_t i = 0; i < peaksAmplitude.size(); i++) {
                peaksEnergyPhysical[i] = peaksAmplitude[i] * fADCtoEnergyFactor;
                peaksEnergySum += peaksEnergyPhysical[i];
            }
        } else {
            // use map to get factor for each channel
            for (size_t i = 0; i < peaksAmplitude.size(); i++) {
                const auto channelId = peaksChannelId[i];
                // check if the channel is in the map
                if (fChannelIDToADCtoEnergyFactor.find(channelId) == fChannelIDToADCtoEnergyFactor.end()) {
                    cerr << "TRestRawPeaksFinderProcess::ProcessEvent: channel id " << channelId
                         << " not found in the map of ADC to energy factors" << endl;
                    exit(1);
                }
                const auto factor = fChannelIDToADCtoEnergyFactor[channelId];

                peaksEnergyPhysical[i] = peaksAmplitude[i] * factor;
                peaksEnergySum += peaksEnergyPhysical[i];
            }
        }

        SetObservableValue("peaksEnergy", peaksEnergyPhysical);
        SetObservableValue("peaksEnergySum", peaksEnergySum);
    }

    // Remove peak-less veto signals after the peak finding if chosen
    if (fRemovePeaklessVetoes && !fRemoveAllVetoes) {
        set<UShort_t> peakSignalIds;
        for (const auto& [channelId, time, amplitude] : eventPeaks) {
            peakSignalIds.insert(channelId);
        }

        vector<UShort_t> signalsToRemove;
        for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
            const auto signal = fInputEvent->GetSignal(signalIndex);
            const UShort_t signalId = signal->GetSignalID();
            const string signalType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);

            if (signalType == "veto" && peakSignalIds.find(signalId) == peakSignalIds.end()) {
                signalsToRemove.push_back(signalId);
            }
        }

        // Now remove all veto signals identified
        for (const auto& signalId : signalsToRemove) {
            fInputEvent->RemoveSignalWithId(signalId);
        }
    }

    // Remove all veto signals after the peak finding if chosen
    if (fRemoveAllVetoes) {
        vector<UShort_t> signalsToRemove;
        for (int signalIndex = 0; signalIndex < fInputEvent->GetNumberOfSignals(); signalIndex++) {
            const auto signal = fInputEvent->GetSignal(signalIndex);
            const UShort_t signalId = signal->GetSignalID();
            const string signalType = fReadoutMetadata->GetTypeForChannelDaqId(signalId);

            if (signalType == "veto") {
                signalsToRemove.push_back(signalId);
            }
        }

        // Now remove all veto signals identified
        for (const auto& signalId : signalsToRemove) {
            fInputEvent->RemoveSignalWithId(signalId);
        }
    }

    return inputEvent;
}

std::map<UShort_t, double> parseStringToMap(const std::string& input) {
    std::map<UShort_t, double> my_map;
    std::string cleaned_input;

    // Step 1: Clean the input to remove curly braces and spaces
    for (char c : input) {
        if (!std::isspace(c) && c != '{' && c != '}') {
            cleaned_input += c;
        }
    }

    // Step 2: Parse each key-value pair
    std::stringstream ss(cleaned_input);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        // Find the colon to split key and value
        size_t colon_pos = pair.find(':');
        if (colon_pos != std::string::npos) {
            // Extract and convert key and value
            int key = std::stoi(pair.substr(0, colon_pos));
            double value = std::stod(pair.substr(colon_pos + 1));
            my_map[key] = value;
        }
    }

    return my_map;
}

void TRestRawPeaksFinderProcess::InitFromConfigFile() {
    const auto filterType = GetParameter("channelType", "");
    if (!filterType.empty()) {
        fChannelTypes.insert(filterType);
    }

    if (fChannelTypes.empty()) {
        // if no channel type is specified, use all channel types
    }

    fThresholdOverBaseline = StringToDouble(GetParameter("thresholdOverBaseline", fThresholdOverBaseline));
    fBaselineRange = Get2DVectorParameterWithUnits("baselineRange", fBaselineRange);
    fDistance = StringToDouble(GetParameter("distance", fDistance));
    fWindow = StringToDouble(GetParameter("window", fWindow));
    fRemoveAllVetoes = StringToBool(GetParameter("removeAllVetos", fRemoveAllVetoes));
    fRemovePeaklessVetoes = StringToBool(GetParameter("removePeaklessVetos", fRemovePeaklessVetoes));

    fTimeBinToTimeFactorMultiplier = GetDblParameterWithUnits("sampling", fTimeBinToTimeFactorMultiplier);
    fTimeBinToTimeFactorOffset = GetDblParameterWithUnits("delay", fTimeBinToTimeFactorOffset);

    fADCtoEnergyFactor = GetDblParameterWithUnits("adcToEnergyFactor", fADCtoEnergyFactor);
    const string fChannelIDToADCtoEnergyFactorAsString = GetParameter("channelIDToADCtoEnergyFactor", "");
    if (!fChannelIDToADCtoEnergyFactorAsString.empty()) {
        // map should be in the format: "{channelId1: factor1, channelId2: factor2, ...}" (spaces are allowed
        // but not required)
        fChannelIDToADCtoEnergyFactor = parseStringToMap(fChannelIDToADCtoEnergyFactorAsString);
    }

    if (fADCtoEnergyFactor != 0 && !fChannelIDToADCtoEnergyFactor.empty()) {
        cerr << "TRestRawPeaksFinderProcess::InitFromConfigFile: both adcToEnergyFactor and "
                "channelIDToADCtoEnergyFactor are defined. Please, remove one of them."
             << endl;
        exit(1);
    }

    if (fBaselineRange.X() > fBaselineRange.Y() || fBaselineRange.X() < 0 || fBaselineRange.Y() < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: baseline range is not sorted or < 0" << endl;
        exit(1);
    }

    if (fThresholdOverBaseline < 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: threshold over baseline is < 0" << endl;
        exit(1);
    }

    if (fDistance <= 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: distance is < 0" << endl;
        exit(1);
    }

    if (fWindow <= 0) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: window is < 0" << endl;
        exit(1);
    }

    if (filterType != "veto" && fRemovePeaklessVetoes) {
        cerr << "TRestRawPeaksFinderProcess::InitProcess: removing veto signals only makes sense when the "
                "process is applied to veto signals. Remove \"removePeaklessVetos\" parameter"
             << endl;
        exit(1);
    }
}

void TRestRawPeaksFinderProcess::PrintMetadata() {
    BeginPrintProcess();

    RESTMetadata << "Applying process to channel types: ";
    for (const auto& type : fChannelTypes) {
        RESTMetadata << type << " ";
    }
    RESTMetadata << RESTendl;

    RESTMetadata << "Baseline range for tpc signals: " << fBaselineRange.X() << " - " << fBaselineRange.Y()
                 << RESTendl;
    RESTMetadata << "Threshold over baseline for veto signals: " << fThresholdOverBaseline << RESTendl;

    RESTMetadata << "Distance: " << fDistance << RESTendl;
    RESTMetadata << "Window: " << fWindow << RESTendl;

    EndPrintProcess();
}
