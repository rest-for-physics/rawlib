//*** A Macro that generates histograms for veto_MaxPeakAmplitude
//*** and veto_PeakTime for each veto channel (signal ID).
//*** Required input is the filename.
//
// Author: Konrad Altenmüller, January 2021
//
//
//
#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TRestRawVetoAnalysisProcess.h>
#include <TRestRun.h>
#include <TRestStringHelper.h>
#include <map>

Int_t REST_Raw_PlotVetoData(string fileName =
                                "/home/konrad/VetoTest9Units/data/"
                                "R01133_000_RawToSignal_Ar2Iso_BackgroundWith9Vetos_konrad_2.3.1.root",
                            int starVal = 0, int endVal = 4000, int bins = 100) {
    TRestRun* run = new TRestRun(fileName);
    TRestAnalysisTree* aTree = run->GetAnalysisTree();
    int n = aTree->GetNbranches();

    // *******************************************************************************
    // Get metadata (signal IDs and veto group names
    // *******************************************************************************

    TRestRawVetoAnalysisProcess* veto =
        (TRestRawVetoAnalysisProcess*)run->GetMetadataClass("TRestRawVetoAnalysisProcess");

    vector<double> signalIddouble;
    vector<double> vetoSignalId = veto->GetVetoSignalIDs();
    vector<string> vetoName;

    // check if signal IDs are defined as a single list or in groups
    if (vetoSignalId[0] == -1.) {
        pair<vector<string>, vector<string>> vetoGroups = veto->GetVetoGroups();
        vector<string> groupNames = std::get<0>(vetoGroups);
        vector<string> groupIds = std::get<1>(vetoGroups);
        for (unsigned int i = 0; i < groupIds.size(); i++) {
            vector<double> id = StringToElements(groupIds[i], ",");
            for (unsigned int j = 0; j < id.size(); j++) {
                signalIddouble.push_back(id[j]);
                vetoName.push_back(groupNames[i]);
            }
        }
    } else
        signalIddouble = vetoSignalId;

    // convert vector from double to int
    vector<int> signalId(signalIddouble.begin(), signalIddouble.end());

    cout << "Signal IDs:";
    for (unsigned int i = 0; i < signalId.size(); i++)
        cout << " " << std::fixed << std::setprecision(0) << signalId[i] << ",";
    cout << '\b' << "." << endl;

    cout << "Veto names:";
    for (unsigned int i = 0; i < signalId.size(); i++) cout << " " << vetoName[i] << ",";
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
    // Iterate over branches to find veto observables
    // *******************************************************************************
    for (unsigned int i = 0; i < n; i++) {
        TString obsName = aTree->GetObservableName(i);

        // ***********************************************************************
        // Peak time histograms
        // ***********************************************************************
        if (obsName.BeginsWith("veto_PeakTime")) {
            cout << "Observable name: " << obsName << endl;

            // Get the observable and initialize a std::map<int,double> to save the values
            int obsID = aTree->GetObservableID(obsName.TString::Data());
            map<int, double> peakTimeMap;
            peakTimeMap.clear();

            // ***************************************************************
            // Loop over signal IDs
            // ***************************************************************
            for (unsigned int j = 0; j < signalId.size(); j++) {
                bool flag = 0;

                // *******************************************************
                // Fill histogram
                // *******************************************************
                for (unsigned int n = 0; n < run->GetEntries(); n++) {
                    run->GetEntry(n);
                    any a = aTree->GetObservable(obsID);
                    a >> peakTimeMap;

                    // Double_t value = aTree->GetObservableValue<Double_t>("veto_PeakTime_ok.second");
                    double value = peakTimeMap[signalId[j]];

                    if (value > 0.1) {
                        peakTimeHist[j]->Fill(value);
                        flag = 1;  // flag, so that this histogram does not get a duplicate at the next
                                   // observable iteration
                    }
                    peakTimeMap.clear();
                }

                // Draw histogram for each signal ID
                // Don't draw, if it exists already
                if (flag) {
                    TVirtualPad* c1 = canvas[j]->cd(1);
                    peakTimeHist[j]->Draw();
                }
            }
        }

        // ***********************************************************************
        // Max peak amplitude histograms
        // ***********************************************************************

        if (obsName.BeginsWith("veto_MaxPeakAmplitude")) {
            cout << "Observable name: " << obsName << endl;

            // Get the observable and initialize a std::map<int,double> to save the values
            int obsID = aTree->GetObservableID(obsName.TString::Data());
            map<int, double> maxPeakAmplitudeMap;
            maxPeakAmplitudeMap.clear();

            // ***************************************************************
            // Loop over signal IDs
            // ***************************************************************
            for (unsigned int j = 0; j < signalId.size(); j++) {
                bool flag = 0;

                // *******************************************************
                // Fill histogram
                // *******************************************************
                for (unsigned int n = 0; n < run->GetEntries(); n++) {
                    run->GetEntry(n);
                    any a = aTree->GetObservable(obsID);
                    a >> maxPeakAmplitudeMap;

                    double value = maxPeakAmplitudeMap[signalId[j]];

                    if (value > 0.1) {
                        maxPeakAmplitudeHist[j]->Fill(value);
                        flag = 1;  // flag, so that this histogram does not get a duplicate at the next
                                   // observable iteration
                    }
                    maxPeakAmplitudeMap.clear();
                }

                // Draw histogram for each signal ID
                // Don't draw, if it exists already
                if (flag) {
                    TVirtualPad* c2 = canvas[j]->cd(2);
                    maxPeakAmplitudeHist[j]->Draw();
                    c2->SetLogy();
                }
            }
        }
    }
    return 0;
}
