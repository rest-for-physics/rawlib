

void REST_Raw_NoiseRMS (const std::string &fileName, int nEvents = 10){


  TRestRun run(fileName);

  TRestRawSignalEvent *rawEvent = (TRestRawSignalEvent*)run.GetInputEvent();

  std::map<int, double > channelMapMean;
  std::map<int, double > channelMapStd;
  std::map<int, int > channelMapCount;

  
  int entries = run.GetEntries();

  nEvents = std::min(nEvents, entries);

  for(int i=0;i<nEvents;i++){
    run.GetEntry(i);
    int chId;
    double mean=0, rms=0;
      for (int s = 0; s < rawEvent->GetNumberOfSignals(); s++) {
        TRestRawSignal* rawSignal = rawEvent->GetSignal(s);
        const int daqChannel = rawSignal->GetSignalID();
          for(int p=0;p<rawSignal->GetNumberOfPoints();p++){
             short data = rawSignal->GetRawData(p);
             channelMapMean[daqChannel] += data;
             channelMapStd[daqChannel] += data*data;
             channelMapCount[daqChannel]++;
          }
      }
  
  }

  auto grE = new TGraphErrors();

  int c =0;
  for(const auto & [channel, count] : channelMapCount){
    if(count>0){
      double mean =  channelMapMean[channel]/count;
      double std = sqrt(channelMapStd[channel]/count - mean*mean );
      std::cout<< "Channel "<<channel<<" Mean "<<mean<<" Std "<<std<<endl;
      grE->SetPoint(c, channel, mean);
      grE->SetPointError(c,0,std);
      c++;
    }

  }

  std::cout<<"Total number of events processed "<<nEvents<<endl;

  grE->Draw("AP");



}

