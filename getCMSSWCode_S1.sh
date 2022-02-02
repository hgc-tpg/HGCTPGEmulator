#!/bin/bash
git init
git remote add -t S1EmulatorDevelopement S1-cmssw git@github.com:portalesHEP/cmssw.git
git config core.sparseCheckout true
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
git pull S1-cmssw S1EmulatorDevelopement
git checkout -b S1EmulatorDevelopement
