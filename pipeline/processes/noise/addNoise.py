#!/usr/bin/python3

import ROOT

ROOT.gSystem.Load("libRestFramework.so")
ROOT.gSystem.Load("libRestRaw.so")

rawEv = ROOT.TRestRawSignalEvent()

signal = ROOT.TRestRawSignal()

for x in range(0, 512):
    signal.AddPoint(0)

rawEv.AddSignal(signal)

addNoiseProcess = ROOT.TRestRawSignalAddNoiseProcess("metadata.rml")

outEv = addNoiseProcess.ProcessEvent(rawEv)

outEv.GetSignal(0).CalculateBaseLine(50, 450)
baseLineSigma = outEv.GetSignal(0).GetBaseLineSigma()

print("\nEvaluating baseline sigma at the output event")
if baseLineSigma < 15 or baseLineSigma > 25:
    print("\nEvaluation failed! Sigma is outside (15,25) range!")
    exit(202)
print("[\033[92m OK \x1b[0m]")

exit(0)
