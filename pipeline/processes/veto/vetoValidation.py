#!/usr/bin/python3

import ROOT

ROOT.gSystem.Load("libRestFramework.so")
ROOT.gSystem.Load("libRestRaw.so")

rawEv = ROOT.TRestRawSignalEvent()

signal1 = ROOT.TRestRawSignal()
signal1.SetID(4688)
signal2 = ROOT.TRestRawSignal()
signal2.SetID(4705)
signal3 = ROOT.TRestRawSignal()
signal3.SetID(4676)
signal4 = ROOT.TRestRawSignal()
signal4.SetID(4)

for x in range(0, 512):
    signal1.AddPoint(0)
    signal2.AddPoint(0)
    signal3.AddPoint(0)
    signal4.AddPoint(0)

rawEv.AddSignal(signal1)
rawEv.AddSignal(signal2)
rawEv.AddSignal(signal3)
rawEv.AddSignal(signal4)

addVetoProcess = ROOT.TRestRawVetoAnalysisProcess("veto.rml")

outEv = addVetoProcess.ProcessEvent(rawEv)

print("\nChecking if observable was created and veto signal was removed")
for i in [1, 2, 3]:
    if outEv.GetSignalIndex(i) != -1:
        print("\nVeto failed: veto signal still present!")
        exit(202)
if outEv.GetSignalIndex(4) == -1:
    print("\nVeto failed: non veto signal removed!")
    exit(202)


print("\nChecking if veto observables were created")

rootfile = ROOT.TFile.Open("R01208_output.root", "READ")
tree = rootfile.Get("AnalysisTree")


if tree.ObservableExists("veto_PeakTime_top") == False:
    print("\nVeto failed: observable not created!")
    exit(202)

if tree.ObservableExists("veto_MaxPeakAmplitude_back") == False:
    print("\nVeto failed: observable not created!")
    exit(202)

print("[\033[92m OK \x1b[0m]")

exit(0)
