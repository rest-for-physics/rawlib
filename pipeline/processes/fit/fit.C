Int_t fit() {
    TRestRawSignalEvent* ev = new TRestRawSignalEvent();

    TRestRawSignal* sgnl = new TRestRawSignal();
    for (int n = 0; n < 512; n++) sgnl->AddPoint(0);
    sgnl->IncreaseBinBy(70, 100);
    ev->AddSignal(*sgnl);

    ///// Initializing processes through metadata definition
    string cfgFile = "metadata.rml";
    TRestRawSignalShapingProcess* shaper = new TRestRawSignalShapingProcess((char*)cfgFile.c_str());
    shaper->PrintMetadata();

    TRestRawSignalAddNoiseProcess* noise = new TRestRawSignalAddNoiseProcess((char*)cfgFile.c_str());
    noise->PrintMetadata();

    TRestRawSignalFittingProcess* fit = new TRestRawSignalFittingProcess((char*)cfgFile.c_str());
    fit->PrintMetadata();

    ///// Processing event
    TRestRawSignalEvent* shapedEvent = (TRestRawSignalEvent*)shaper->ProcessEvent(ev);

    TRestRawSignalEvent* noisyEvent = (TRestRawSignalEvent*)noise->ProcessEvent(shapedEvent);

    TRestRawSignalEvent* fittedEvent = (TRestRawSignalEvent*)fit->ProcessEvent(noisyEvent);
    
    
    TF1* f = new TF1("fit1",
                         "[0]+[1]*TMath::Exp(-3. * (x-[3])/[2]) * "
                         "(x-[3])/[2] * (x-[3])/[2] * (x-[3])/[2] * "
                         "sin((x-[3])/[2])/(1+TMath::Exp(-10000*(x-[3])))", 0, 511);
    
    
    cout << fit->GetBaseline() << endl;
    cout << fit->GetAmplitude() << endl;
    cout << fit->GetShaping() << endl;
    cout << fit->GetStartPosition() << endl;

    /// Debugging output
    TCanvas* c = new TCanvas();
    noisyEvent->DrawEvent();
    // shapedEvent->DrawEvent();
    f->SetParameters(fit->GetBaseline(), fit->GetAmplitude(), fit->GetShaping(), fit->GetStartPosition());
    f->Draw("same");
    f->SetLineColor(kOrange);
    
    // Before noise
    TCanvas* c2 = new TCanvas();
    TRestRawSignal* singleSignal = shapedEvent->GetSignal(0);  
    Int_t nBins = singleSignal->GetNumberOfPoints();
    TH1D* h = new TH1D("histo", "", nBins, 0, nBins);

    for (int i = 0; i < nBins; i++) {
        h->Fill(i, singleSignal->GetRawData(i) - f->Eval(i));
    }
    h->Draw("hist");


    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
