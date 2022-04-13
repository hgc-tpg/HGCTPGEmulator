#!bin/bash

### Running on "S1-format" inputs
TC_LIST="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TClist.json"
#TC_MAP="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TCmap_rzphi.csv"
#CONFIG="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/config_S1SortingTruncation_v0_20211129.json"
CONFIG="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/geometryConfig_backendStage1.json"

OUTDIR="./"

./bin/HGCTPGStage1EmulatorTester.exe $TC_LIST $CONFIG $OUTDIR

