#!/bin/bash

CMSSW_VERSION=${1-CMSSW_12_3_0}
SCRAMARCH=${2-slc7_amd64_gcc10}

# setup CMS environment to get required C++17 version
source /cvmfs/cms.cern.ch/cmsset_default.sh
export SCRAM_ARCH=${SCRAMARCH}
cmsrel ${CMSSW_VERSION}
cd ${CMSSW_VERSION}/src/
cmsenv
cd -

git init
git remote add -t S1EmulatorDevelopement_pre4 S1-cmssw https://github.com/portalesHEP/cmssw.git
git config core.sparseCheckout true
echo L1Trigger/L1THGCal/interface/backend_emulator >> .git/info/sparse-checkout
echo L1Trigger/L1THGCal/src/backend_emulator >> .git/info/sparse-checkout
git pull S1-cmssw S1EmulatorDevelopement_pre4
git checkout -b S1EmulatorDevelopement_pre4

