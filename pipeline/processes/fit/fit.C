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

    /// Debugging output
    TCanvas* c = new TCanvas("c1", "", 800, 600);
    // shapedEvent->DrawEvent();
    noisyEvent->DrawEvent();

    cout << "[\033[92m OK \x1b[0m]" << endl;
    return 0;
}
