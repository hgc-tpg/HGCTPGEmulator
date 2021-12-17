#!/bin/bash
git init
git remote add -t S2ClusteringEmulatorIntegration S2-cmssw git@github.com:EmyrClement/cmssw.git
git remote add -t S1SortingTruncationEmulator S1-cmssw git@github.com:portalesHEP/cmssw.git
git config core.sparseCheckout true
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
git pull S2-cmssw S2ClusteringEmulatorIntegration
git pull S1-cmssw S1SortingTruncationEmulator
#git checkout -b S1SortingTruncationEmulator
