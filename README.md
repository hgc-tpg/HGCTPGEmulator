# HGCTPGEmulator

## Overview

Ensemble of python and C++ modules to run the HGC-TPG backend firmware emulator.

## Dependencies

The testing framework uses the `nlohmann_json` library, which must be installed and accessible. If not, it can be easily set, e.g. in a conda environment:

```
conda create --name newEnv
conda activate newEnv
conda install -c conda-forge nlohmann_json pkg-config
```

`pkg-config` is also needed inside the conda environment, in order to automatically pick up the package location inside the Makefile.

## Setup and run tests

# Setup
```
# get HGCTPGEmulator & CMSSW dependancies
mkdir HGCTPGEmulator
cd HGCTPGEmulator
git init
mv .git/ .githgctpg
git --git-dir=.githgctpg remote add -t HGCTPGEmulator_S1 S1Emulator git@github.com:portalesHEP/HGCTPGEmulator.git
git --git-dir=.githgctpg pull S1Emulator HGCTPGEmulator_S1
source getCMSSWCode_S1.sh

# Create conda env with required packages
conda create --name myEnv
conda activate newEnv
conda install -c conda-forge nlohmann_json pkg-config
```

# run

Note: The paths to input and configuration files found in `run.sh` requires access to the eos repository. If not accessible, copies of these files are available at `/afs/cern.ch/user/l/lportale/public/HGCTPG/`.

```
# Compile & run tester
make
cd test
source run.sh
```

## Stage1 emulator

Temporary input and config files are available at the following link:

https://cernbox.cern.ch/index.php/s/mLpzkHbba8dXgDD

The stage 1 emulator can be tested with two different formats of inputs, as provided here:

- `inputs/inputs.txt`: default input format used in S2 emulation.

  These files directly contain the TC information needed to run the emulator.

- `inputs/TClist.json`: list generated from `ModXXXX_stimuli.txt` files, containing input TCs per module, as used for S1 firwmare testing. 

  For these, additional information on the mapping of FPGA/modules/TCs in <img src="https://render.githubusercontent.com/render/math?math=(r/z,\phi)"> is needed. These are `.csv` files, generated with `python/generateTCMap.py`.

  The map generator relies on the configuration files found in the `data` folder, and correspond to the current geometry used for S1 firmware tests.

## Running Stage 1 tests

  The algorithm implementations are found in `L1THGCal/L1Trigger/src/backend_emulator`. These should ultimately be common with the actual CMSSW implementation (synchronisation to be implemented).

  Additional functionalities are implemented in `HGCTPGEmulator/src/SAConfigParser.cc`, in order to parse input and configuration files, and to be able to modify them on the fly if needed.

  To compile all the components required for tests, simply call `make` from this directory. The executable, `HGCTPGEmulatorTester_S1` will be located in `test`, and some basic configurations to run it are available in `test/run.sh`.

  Right now, `HGCTPGEmulatorTester_S1` expects either two or three input parameters: either a list of TCs in the "S2 format" and a configuration file (example here: `inputs/config_S1SortingTruncation_v0_20211129.cfg`), or a list of TCs in the "S1 format", with the same configuration file, in addition to the `.csv` TC map generated with `python/generateTCMap.py`.
