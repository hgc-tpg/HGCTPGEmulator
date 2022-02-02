#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationImpl_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"

#include "HGCTPGEmulator/interface/SAConfigParser.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <memory>

using namespace l1thgcfirmware;

std::vector<HGCalTriggerCell> triggerCellInput( std::vector< std::vector< HGCalTriggerCell > >& inputs ) {

  std::vector<HGCalTriggerCell> triggerCellsIn;
  for (unsigned int iFrame = 0; iFrame < inputs.size(); ++iFrame ) {
    for (unsigned int iInput = 0; iInput < inputs[iFrame].size(); ++iInput ) {
      auto& tc = inputs[iFrame][iInput];
      tc.setIndex(iInput);
      tc.setClock(iFrame+1);
      if ( tc.dataValid() ) {
        triggerCellsIn.push_back(tc);
      }
    }
  }
  return triggerCellsIn;
}

int main(int argc, char **argv) {

  SAConfigParser cfgReader;

  // parse TC list
  std::vector < std::vector < HGCalTriggerCell > > allTCs = (argc==3 ?
                                                             cfgReader.parseTClist(argv[1]) :
                                                             cfgReader.parseTClist(argv[1],argv[2]));

  // parse config
  const std::string cfgfile = (argc==3 ? argv[2] : argv[3]);
  Stage1TruncationConfig config = cfgReader.parseCfg(cfgfile);

  //	create algo instance
  HGCalStage1TruncationImplSA theAlgo;

  // Outputs
  std::ofstream inputTCsFile;
  inputTCsFile.open("outputs/inputTCs.txt");
  std::ofstream sortedTruncatedTCsFile;
  sortedTruncatedTCsFile.open("outputs/sortedTruncatedTCs.txt");

  unsigned int counter = 0;
  for (const auto TCs : allTCs) {

    inputTCsFile << "Event " << counter << std::endl;
    sortedTruncatedTCsFile << "Event " << counter << std::endl;

    // Run the algorithm
    HGCalTriggerCellSACollection tcs_out_SA;
    unsigned error_code = theAlgo.run(TCs, config, tcs_out_SA);

    std::cout << "TCs.at("<<counter<<").size() = " << TCs.size() << " | "
              << "tcs_out_SA.at("<<counter<<").size() = " << tcs_out_SA.size() << std::endl;
    // fill the input txt files
    for (const auto& tc : TCs ) {

      // get roz bin:
      double rzmin = 0.07587128 *4096/0.7; // magic numbers
      double rzmax = 0.55508006 *4096/0.7; // magic numbers
      double rz_bin_size = (rzmax - rzmin) * 1.001 / 42. ;
      double rz = tc.rOverZ();
      rz = (rz < rzmin ? rzmin : rz);
      rz = (rz > rzmax ? rzmax : rz);
      unsigned rzbin = (rz_bin_size > 0. ? unsigned((rz - rzmin) / rz_bin_size) : 0);

      inputTCsFile << "TCID" << tc.index() << " : "
		   << "roverZ = " << tc.rOverZ() << " "
		   << "(bin " << rzbin << ") :"
		   << tc.energy() << std::endl;
    }
    // fill the output txt files
    for (const auto& tc : tcs_out_SA ) {

      // get roz bin:
      double rzmin = 0.07587128 *4096/0.7; // magic numbers
      double rzmax = 0.55508006 *4096/0.7; // magic numbers
      double rz_bin_size = (rzmax - rzmin) * 1.001 / 42. ;
      double rz = tc.rOverZ();
      rz = (rz < rzmin ? rzmin : rz);
      rz = (rz > rzmax ? rzmax : rz);
      unsigned rzbin = (rz_bin_size > 0. ? unsigned((rz - rzmin) / rz_bin_size) : 0);

      sortedTruncatedTCsFile << "TCID" << tc.index() << " : "
			     << "roverZ = " << tc.rOverZ() << " "
			     << "(bin " << rzbin << ") :"
			     << tc.energy() << std::endl;
    }
    ++counter;
  }
  inputTCsFile.close();
  sortedTruncatedTCsFile.close();
}
