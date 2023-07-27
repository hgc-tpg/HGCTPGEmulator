#!/bin/bash
git init
git remote add -t S2ClusteringEmulatorIntegration my-cmssw git@github.com:EmyrClement/cmssw.git
git config core.sparseCheckout true
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
git pull my-cmssw S2ClusteringEmulatorIntegration
git checkout -b S2ClusteringEmulatorIntegration
