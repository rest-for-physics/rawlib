Int_t commonNoise() {
    TRestRawSignalEvent* ev = new TRestRawSignalEvent();

    TRestRawSignal* sgnl = new TRestRawSignal();
    for (int n = 0; n < 512; n++) sgnl->AddPoint((Short_t)(50 * TMath::Sin(2 * TMath::Pi() * n / 200)));

    for (int n = 0; n < 20; n++) {
        sgnl->SetID(n);
        ev->AddSignal(*sgnl);
    }

    string cfgFile = "metadata.rml";
    // Initializing processes
    TRestRawSignalAddNoiseProcess* addNoise = new TRestRawSignalAddNoiseProcess((char*)cfgFile.c_str());
    addNoise->PrintMetadata();

    TRestRawCommonNoiseReductionProcess* commonNoise =
        new TRestRawCommonNoiseReductionProcess((char*)cfgFile.c_str());
    commonNoise->PrintMetadata();

    TRestRawSignalEvent* noisyEvent = (TRestRawSignalEvent*)addNoise->ProcessEvent(ev);
    TRestRawSignalEvent* cleanEvent = (TRestRawSignalEvent*)commonNoise->ProcessEvent(noisyEvent);

    // TCanvas* c = new TCanvas("c1", "", 640, 320);
    // cleanEvent->DrawEvent();

    // TCanvas* c2 = new TCanvas("c2", "", 640, 320);
    // noisyEvent->DrawEvent();

    noisyEvent->GetSignal(0)->CalculateBaseLine(50, 540);
    Double_t bLineSigmaNoisy = noisyEvent->GetSignal(0)->GetBaseLineSigma();

    cout << "BL(noisy): " << bLineSigmaNoisy << endl;

    cleanEvent->GetSignal(0)->CalculateBaseLine(50, 540);
    Double_t bLineSigmaClean = cleanEvent->GetSignal(0)->GetBaseLineSigma();

    cout << "BL(clean): " << bLineSigmaClean << endl;

    if (bLineSigmaClean > 200) {
        cout << "The base line sigma clean is higher than 200!" << endl;
        return 1;
    }

    if (bLineSigmaNoisy < 1000) {
        cout << "The base line sigma clean is lower than 1000!" << endl;
        return 2;
    }

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
