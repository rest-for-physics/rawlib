// A Macro that generates histograms for veto_MaxPeakAmplitude and veto_PeakTime for each veto channel (signal
// ID). Required input is the filename.
//
// Author: Konrad Altenmüller, May 2021
//
//
//
#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TRestRawVetoAnalysisProcess.h>
#include <TRestRun.h>
#include <TRestStringHelper.h>
#include <string.h>

#include <map>

Int_t REST_Raw_PlotVetoData(
    string fileName = "data/R01133_000_RawToSignal_Ar2Iso_BackgroundWith9Vetos_konrad_2.3.1.root",
    string group = "all", string processName = "veto", int starVal = 0, int endVal = 4000, int bins = 100) {
    TRestRun* run = new TRestRun(fileName);
    TRestAnalysisTree* aTree = run->GetAnalysisTree();
    int n = aTree->GetNbranches();

    // *******************************************************************************
    // Get Metadata
    // *******************************************************************************

    TRestRawVetoAnalysisProcess* veto =
        (TRestRawVetoAnalysisProcess*)run->GetMetadataClass("TRestRawVetoAnalysisProcess");

    vector<double> signalIddouble;
    vector<double> vetoSignalId = veto->GetVetoSignalIDs();
    vector<string> vetoName;
    vector<string> groupNames;
    vector<string> groupIds;

    // check if signal IDs are defined as a single list or in groups
    // if vetoes are defined in groups:
    if (vetoSignalId[0] == -1.) {
        // check if all groups or only a selected group should be plotted
        if (group == "all") {
            pair<vector<string>, vector<string>> vetoGroups = veto->GetVetoGroups();
            groupNames = std::get<0>(vetoGroups);
            groupIds = std::get<1>(vetoGroups);
        } else {
            groupNames.push_back(group);
            groupIds.push_back(veto->GetGroupIds(group));
            // In case group name doesn't exist:
            if (std::strcmp(string(groupIds[0]).c_str(), string("-1").c_str()) == 0) {
                cout << "\nERROR: specified veto group does not exist!\n" << endl;
                return 0;
            }
        }
        // create vector of group name with same length as signal IDs (to print it out below)
        for (unsigned int i = 0; i < groupIds.size(); i++) {
            vector<double> id = StringToElements(groupIds[i], ",");
            // nVeto.push_back(id.size());
            for (unsigned int j = 0; j < id.size(); j++) {
                signalIddouble.push_back(id[j]);
                vetoName.push_back(groupNames[i]);
            }
        }

        // if vetoes are defined in a single list:
    } else {
        signalIddouble = vetoSignalId;
        groupNames.push_back("standard");
        for (unsigned int i = 0; i < signalIddouble.size(); i++) vetoName.push_back("standard");
    }

    // convert ID vector from double to int
    vector<int> signalId(signalIddouble.begin(), signalIddouble.end());

    // Print Names and IDs
    cout << "Signal IDs:\tVeto names:\n";
    for (unsigned int i = 0; i < signalId.size(); i++)
        cout << " " << std::fixed << std::setprecision(0) << signalId[i] << " -  -  -  -  - " << vetoName[i]
             << "\n";
    cout << '\b' << "." << endl;

    // *******************************************************************************
    // Create canvases and histograms for each signalId
    // *******************************************************************************

    vector<TH1D*> peakTimeHist;
    vector<TH1D*> maxPeakAmplitudeHist;
    vector<TCanvas*> canvas;

    for (unsigned int i = 0; i < signalId.size(); i++) {
        string peakTimeHistName;
        string maxPeakAmplitudeHistName;

        if (vetoName.empty()) {
            peakTimeHistName = "Peak time: signal ID " + std::to_string(signalId[i]);
            maxPeakAmplitudeHistName = "Max peak amp.: signal ID " + std::to_string(signalId[i]);
        } else {
            peakTimeHistName =
                "Peak time: signal ID " + std::to_string(signalId[i]) + " (" + vetoName[i] + ")";
            maxPeakAmplitudeHistName =
                "Max peak amp.: signal ID " + std::to_string(signalId[i]) + " (" + vetoName[i] + ")";
        }

        TH1D* h1 = new TH1D(peakTimeHistName.c_str(), peakTimeHistName.c_str(), bins, starVal, 500);
        TH1D* h2 = new TH1D(maxPeakAmplitudeHistName.c_str(), maxPeakAmplitudeHistName.c_str(), bins, starVal,
                            endVal);

        peakTimeHist.push_back(h1);
        maxPeakAmplitudeHist.push_back(h2);

        int width = 1200;
        int height = 500;
        TCanvas* c = new TCanvas();
        c->SetCanvasSize(width, height);
        c->SetWindowSize(width + 4, height + 28);
        c->Divide(2, 1);
        canvas.push_back(c);
    }

    // *******************************************************************************

    string obsName;
    vector<string> obsNameTime;
    vector<string> obsNameAmp;
    int obsID;
    map<int, double> peakTimeMap;
    map<int, double> maxPeakAmplitudeMap;
    vector<double> id;
    double value_time;

    // construct observable names
    if (vetoSignalId[0] == -1) {
        for (unsigned int i = 0; i < groupNames.size(); i++) {
            obsName = processName + "_PeakTime_" + groupNames[i];
            obsNameTime.push_back(obsName);
            obsName = processName + "_MaxPeakAmplitude_" + groupNames[i];
            obsNameAmp.push_back(obsName);
        }
    } else {
        obsNameTime.push_back(processName + "_PeakTime");
        obsNameAmp.push_back(processName + "_MaxPeakAmplitude");
    }

    // *******************************************************************************
    // Iterate over entries
    // *******************************************************************************

    for (unsigned int n = 0; n < run->GetEntries(); n++) {
        run->GetEntry(n);
        int nHist = 0;
        // iterate over observables
        for (unsigned int i = 0; i < groupNames.size(); i++) {
            // cout << "Observable Name: " << obsNameTime[i] << "\n";

            // ***********************
            // Peak Time Histograms
            // ***********************
            obsID = aTree->GetObservableID(obsNameTime[i]);
            peakTimeMap.clear();

            auto a = aTree->GetObservable(obsID);

            a >> peakTimeMap;

            id.clear();
            // if vetoes are defined in groups
            if (vetoSignalId[0] == -1) {
                id = StringToElements(groupIds[i], ",");
            } else {
                id = vetoSignalId;
            }

            // iterate over signal IDs
            for (unsigned int j = 0; j < id.size(); j++) {
                //	cout << "signal ID: " << id[j] << "\n";
                value_time = peakTimeMap[id[j]];
                // put in correct histogram
                if (value_time > 0.1) {
                    peakTimeHist[nHist + j]->Fill(value_time);
                }
            }

            // ***********************
            // Max Peak Amplitude Histrograms
            // ***********************
            obsID = aTree->GetObservableID(obsNameAmp[i]);
            maxPeakAmplitudeMap.clear();

            a = aTree->GetObservable(obsID);
            a >> maxPeakAmplitudeMap;

            id.clear();
            // if vetoes are defined in groups
            if (vetoSignalId[0] == -1) {
                id = StringToElements(groupIds[i], ",");
            } else {
                id = vetoSignalId;
            }
            // iterate over signal IDs
            for (unsigned int j = 0; j < id.size(); j++) {
                double value_amp = maxPeakAmplitudeMap[id[j]];
                // put in correct histogram
                if (value_time > 0.1) {
                    maxPeakAmplitudeHist[nHist + j]->Fill(value_amp);
                }
            }
            nHist += id.size();
        }
    }

    // Draw the histograms in correct canvas
    for (unsigned int i = 0; i < canvas.size(); i++) {
        TVirtualPad* c1 = canvas[i]->cd(1);
        peakTimeHist[i]->Draw();

        TVirtualPad* c2 = canvas[i]->cd(2);
        maxPeakAmplitudeHist[i]->Draw();
        c2->SetLogy();
    }

    return 0;
}
