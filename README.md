# HGCTPGEmulator

## Overview

Ensemble of python and C++ modules to run the HGC-TPG backend firmware emulator.

## Stage1 emulator

Temporary input and config files are available at the following link:

https://cernbox.cern.ch/index.php/s/mLpzkHbba8dXgDD



The stage 1 emulator can be tested with two different formats of inputs, as provided here:

- `inputs/inputs.txt`: default input format used in S2 emulation.

  These files directly contain the TC information needed to run the emulator.

- `inputs/TClist.txt`: list generated from `ModXXXX_stimuli.txt` files, containing input TCs per module, as used for S1 firwmare testing. 

  For these, additional information on the mapping of FPGA/modules/TCs in <img src="https://render.githubusercontent.com/render/math?math=(r/z,\phi)"> is needed. These are `.csv` files, generated with `python/generateTCMap.py`.

  The map generator relies on the configuration files found in the `data` folder, and correspond to the current geometry used for S1 firmware tests.

## Running Stage 1 tests

  The algorithm implementations are found in `L1THGCal/L1Trigger/src/backend_emulator`. These should ultimately be common with the actual CMSSW implementation (synchronisation to be implemented).

  Additional functionalities are implemented in `HGCTPGEmulator/src/SAConfigParser.cc`, in order to parse input and configuration files, and to be able to modify them on the fly if needed.

  To compile all the components required for tests, simply call `make` from this directory. The executable, `HGCTPGEmulatorTester_S1` will be located in `test`, and some basic configurations to run it are available in `test/run.sh`.

  Right now, `HGCTPGEmulatorTester_S1` expects either two or three input parameters: either a list of TCs in the "S2 format" and a configuration file (example here: `inputs/config_S1SortingTruncation_v0_20211129.cfg`), or a list of TCs in the "S1 format", with the same configuration file, in addition to the `.csv` TC map generated with `python/generateTCMap.py`.
