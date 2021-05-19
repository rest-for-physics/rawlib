// A Macro that generates histograms for veto_MaxPeakAmplitude and veto_PeakTime for each veto channel (signal ID). Required input is the filename.
//
// Author: Konrad Altenmüller, May 2021
//
//
//
#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TRestRun.h>
#include <map>
#include <TRestRawVetoAnalysisProcess.h>
#include <TRestStringHelper.h>
#include <chrono>

Int_t REST_Raw_PlotVetoData(string fileName = "data/R01133_000_RawToSignal_Ar2Iso_BackgroundWith9Vetos_konrad_2.3.1.root", string group="all", int starVal = 0, int endVal = 4000, int bins = 100){

	
	// initiate clock for testing
	// using std::chrono::high_resolution_clock;
	// using std::chrono::duration_cast;
	// using std::chrono::duration;
	// using std::chrono::seconds;	

	// auto t0 = high_resolution_clock::now();	
	
	TRestRun* run = new TRestRun(fileName);
	TRestAnalysisTree* aTree = run->GetAnalysisTree();
	int n = aTree->GetNbranches();

	// *******************************************************************************
	// Get Metadata
	// *******************************************************************************

	TRestRawVetoAnalysisProcess* veto = (TRestRawVetoAnalysisProcess* ) run->GetMetadataClass("TRestRawVetoAnalysisProcess");

	vector<double> signalIddouble;
	vector<double> vetoSignalId = veto->GetVetoSignalIDs();
	vector<string> vetoName;
	vector<string> groupNames;
	vector<string> groupIds;
	
	// check if signal IDs are defined as a single list or in groups
	if (vetoSignalId[0] == -1.){
		pair<vector<string>,vector<string>> vetoGroups = veto->GetVetoGroups();
		groupNames = std::get<0>(vetoGroups);
		groupIds   = std::get<1>(vetoGroups);
		if (group !="all"){
			if (std::find(groupNames.begin(),groupNames.end(),group) != groupNames.end()){
				ptrdiff_t pos = distance(groupNames.begin(), find(groupNames.begin(), groupNames.end(), group));			
				groupNames.clear();
				groupNames.push_back(group);
				string temp = groupIds[pos];	
				groupIds.clear();
				groupIds.push_back(temp);
			} else {
			cout << "\nERROR: specified veto group does not exist!\n" << endl;
			return 0;
			}	
		
		}

		for (unsigned int i=0; i<groupIds.size(); i++){
			vector<double> id  =  StringToElements(groupIds[i],",");
			// nVeto.push_back(id.size());
			for (unsigned int j=0; j<id.size(); j++){
				signalIddouble.push_back(id[j]);
				vetoName.push_back(groupNames[i]);
			}	
		}
	}
	else signalIddouble = vetoSignalId;


	// convert vector from double to int
	vector<int> signalId(signalIddouble.begin(), signalIddouble.end());

	cout << "Signal IDs:";
	for (unsigned int i=0; i<signalId.size(); i++)
		cout << " " << std::fixed <<  std::setprecision(0) << signalId[i] << ",";
	cout << '\b' << "." << endl;
	
	cout << "Veto names:";
	for (unsigned int i=0; i<signalId.size(); i++)
		cout << " " << vetoName[i] << ",";
	cout << '\b' << "." << endl;
	
	// *******************************************************************************
	// Create canvases and histograms for each signalId
	// *******************************************************************************

	// auto t1 = high_resolution_clock::now();
	// duration<double> s_double = t1 - t0;
	// cout << "start up duration = " << s_double.count() << "s\n";

	vector<TH1D*> peakTimeHist;
	vector<TH1D*> maxPeakAmplitudeHist;
	vector<TCanvas*> canvas;

	for (unsigned int i=0; i<signalId.size(); i++){

		string peakTimeHistName;
		string maxPeakAmplitudeHistName;	

		if (vetoName.empty()){
			peakTimeHistName = "Peak time: signal ID "+std::to_string(signalId[i]);
			maxPeakAmplitudeHistName = "Max peak amp.: signal ID "+std::to_string(signalId[i]);
		} else { 
			peakTimeHistName = "Peak time: signal ID "+std::to_string(signalId[i]) + " ("+vetoName[i]+")";
			maxPeakAmplitudeHistName = "Max peak amp.: signal ID "+std::to_string(signalId[i]) + " ("+vetoName[i]+")";
		}
		
		TH1D* h1 = new TH1D(peakTimeHistName.c_str(),peakTimeHistName.c_str(),bins,starVal,500);
		TH1D* h2 = new TH1D(maxPeakAmplitudeHistName.c_str(),maxPeakAmplitudeHistName.c_str(),bins,starVal,endVal);

		peakTimeHist.push_back(h1);
		maxPeakAmplitudeHist.push_back(h2);

		int width = 1200;
		int height= 500;
		TCanvas* c = new TCanvas();
		c->SetCanvasSize(width,height);
		c->SetWindowSize(width+4,height+28);
		c->Divide(2,1);
		canvas.push_back(c);
	}


	// t1 = high_resolution_clock::now();
	// s_double = t1 - t0;
	// cout << "start to iterate over branches = " << s_double.count() << "s\n";
	
	

	// *******************************************************************************
	
	string obsName;
	vector<string> obsNameTime;
	vector<string> obsNameAmp;
	int obsID;
	map<int,double> peakTimeMap; 
	map<int,double> maxPeakAmplitudeMap; 
	vector<double> id;
	double value_time;

	// construct observable names 
	for (unsigned int i=0; i<groupNames.size(); i++){	
		obsName = "veto_PeakTime_"+groupNames[i];
		obsNameTime.push_back(obsName);
		obsName = "veto_MaxPeakAmplitude_"+groupNames[i];
		obsNameAmp.push_back(obsName);
	}

	// *******************************************************************************
	// Iterate over entries
	// *******************************************************************************

	// *** debug
	
	// TH1F *source = new TH1F("source","source hist",100,-3,3);
	// source->FillRandom("gaus",1000);
	// TH1F *final = new TH1F("final","final hist",100,-3,3);
	// ***

	for (unsigned int n=0; n<run->GetEntries(); n++){
		run->GetEntry(n);
		int nHist = 0;
		// iterate over observables
		for (unsigned int i=0; i<groupNames.size(); i++){
			// cout << "Observable Name: " << obsNameTime[i] << "\n";
		
			// Peak Time
			obsID = aTree->GetObservableID(obsNameTime[i]);
			peakTimeMap.clear();
			
			any a = aTree->GetObservable(obsID);
			a >> peakTimeMap;

			id.clear();
			id  =  StringToElements(groupIds[i],",");
			// iterate over signal IDs
			for (unsigned int j=0; j<id.size(); j++){
			//	cout << "signal ID: " << id[j] << "\n";
				value_time = peakTimeMap[id[j]];
				// put in correct histogram
				if (value_time > 0.1){
					peakTimeHist[nHist+j]->Fill(value_time); //j ist falsch --> + anzahl an vorherigen histogrammen muss draufgerechnet werden
				}
				// int nentries = peakTimeHist[j]->GetEntries();
				// cout << nentries << "\n";
			}
	
			// Max Peak Amplitude
			obsID = aTree->GetObservableID(obsNameAmp[i]);
			maxPeakAmplitudeMap.clear();
			
			a = aTree->GetObservable(obsID);
			a >> maxPeakAmplitudeMap;

			id.clear();
			id  =  StringToElements(groupIds[i],",");
			// iterate over signal IDs
			for (unsigned int j=0; j<id.size(); j++){
				//cout << "signal ID: " << id[j] << "\n";
				double value_amp = maxPeakAmplitudeMap[id[j]];
				// put in correct histogram
				if (value_time>0.1){
					maxPeakAmplitudeHist[nHist+j]->Fill(value_amp);
				}

			}
			
			nHist += id.size();

			// ***debug
			// final->Fill(source->GetRandom());
			// ***
		
		}

	}


	// Draw the histograms in correct canvas
	for (unsigned int i=0; i<canvas.size(); i++){
		TVirtualPad* c1 = canvas[i]->cd(1);
		// source->Draw();
		peakTimeHist[i]->Draw();
		// int nEntries = peakTimeHist[i]->GetEntries();
		// cout << "N Entries =" << nEntries << "\n";

		TVirtualPad* c2 = canvas[i]->cd(2);
		maxPeakAmplitudeHist[i]->Draw();
		// final->Draw();
		c2->SetLogy();
	}





	return 0;
}
