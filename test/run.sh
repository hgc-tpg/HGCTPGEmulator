#!bin/bash

### Running on "S1-format" inputs
TC_LIST="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TClist.txt"
TC_MAP="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TCmap_rzphi.csv"
CONFIG="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/config_S1SortingTruncation_v0_20211129.cfg"
./HGCTPGEmulatorTester_S1 $TC_LIST $TC_MAP $CONFIG

### Running on "S2-format" inputs
#TC_LIST="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/inputs.txt"
#CONFIG="/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/config_S1SortingTruncation_v0_20211129.cfg"
#./HGCTPGEmulatorTester_S1 $TC_LIST $CONFIG
