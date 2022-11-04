#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationImpl_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"

#include "HGCTPGStage1Emulator/interface/SAConfigParser.h"
#include "HGCTPGStage1Emulator/interface/SATCParser.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <memory>

using namespace l1thgcfirmware;

typedef std::vector < HGCalTriggerCell > HGCalTriggerCells;
typedef std::vector < HGCalTriggerCells > HGCalTriggerCellsPerBx;

class HGCTPGEmulatorTester {

public:

  HGCTPGEmulatorTester(const std::string, const std::string);
  ~HGCTPGEmulatorTester(){};
  
  int runAlgo(const HGCalTriggerCellsPerBx&, HGCalTriggerCellsPerBx&);
  int dumpTCs(const HGCalTriggerCellsPerBx&, const std::string);
  int dumpTCs_fw(const HGCalTriggerCellsPerBx&, const std::string, const bool);

  //getter
  HGCalTriggerCellsPerBx getTCs() {return allTCs_;}

private:
  // dummy initialization of configuration; overwritten later
  Stage1TruncationConfig stage1Config_ = Stage1TruncationConfig(false,
								0.,
								0.,
								0,
								std::vector<unsigned>{0},
								std::vector<double>{0.});

  double rzmin_;
  double rzmax_;
  double rz_bin_size_;
  static constexpr double margin_ = 1.001;

  //TCMap theTCMap_;
  HGCalStage1TruncationImplSA stage1Algo_;

  HGCalTriggerCellsPerBx allTCs_;

  static constexpr unsigned offset_roz_ = 1;
  static constexpr unsigned mask_roz_ = 0x3f;  // 6 bits, max 64 bins
  static constexpr unsigned mask_phi_ = 1;

  static constexpr unsigned offset_e_ = 8; // energy on 8 bits (0->255)
  
};

HGCTPGEmulatorTester::HGCTPGEmulatorTester(const std::string jsonConfigFile, 
					   const std::string tcList)
{
  
  
  std::cout << "-- Initializing config reader" << std::endl;
  SAConfigParser cfgReader(jsonConfigFile);
  stage1Config_ = cfgReader.getCfg();

  rzmin_ = stage1Config_.rozMin();
  rzmax_ = stage1Config_.rozMax();
  rz_bin_size_ = (rzmax_ - rzmin_) * margin_ / stage1Config_.rozBins();

  std::cout << "-- Initializing TC reader" << std::endl;
  SATCParser tcReader(tcList, cfgReader.getTCmap());
  std::cout << "-- Getting TC list" << std::endl;
  allTCs_ = tcReader.TClist();

}

int HGCTPGEmulatorTester::runAlgo(const HGCalTriggerCellsPerBx& tcPerBx, HGCalTriggerCellsPerBx& sortedTruncatedTCsPerBx) {
  
  unsigned int counter = 0;
  for (const auto TCs : tcPerBx) {
    // Run the algorithm
    HGCalTriggerCells sortedTruncatedTCs;
    unsigned error_code = stage1Algo_.run(TCs, stage1Config_, sortedTruncatedTCs);
    sortedTruncatedTCsPerBx.push_back(sortedTruncatedTCs);
  }
  return 0;
}

int HGCTPGEmulatorTester::dumpTCs(const HGCalTriggerCellsPerBx& tcPerBx,
				  const std::string outputFileName) {
  
  std::ofstream outputStream;
  outputStream.open(outputFileName);
  unsigned int counter = 0;
  for (const auto TCs: tcPerBx) {
    outputStream << "Event " << counter << std::endl;
    for (const auto& tc : TCs ) {

      double rz = tc.rOverZ();
      rz = (rz < rzmin_ ? rzmin_ : rz);
      rz = (rz > rzmax_ ? rzmax_ : rz);
      unsigned rzbin = (rz_bin_size_ > 0. ? unsigned((rz - rzmin_) / rz_bin_size_) : 0);
      
      outputStream << "TCID" << tc.index() << " : "
		   << "roverZ = " << tc.rOverZ() << " "
		   << "(bin " << rzbin << ") :"
		   << tc.energy() << std::endl;
    }
    counter++;
  }
  outputStream.close();
  return 0;
}

int HGCTPGEmulatorTester::dumpTCs_fw(const HGCalTriggerCellsPerBx& tcPerBx,
				     const std::string outputFileName,
				     const bool dumpAddresses) {  

  /* 

     1 file per (roz,phi) bin (a,b): results_Phi(a)Bin(b).txt

     in file:
     
      ---- TCs ---->
    | XXXXYY XXXXYY XXXXYY ... 
    | XXXXYY XXXXYY XXXXYY ... 
    B XXXXYY XXXXYY XXXXYY ... 
    x XXXXYY XXXXYY XXXXYY ... 
    | XXXXYY XXXXYY XXXXYY ... 
    | XXXXYY XXXXYY XXXXYY ... 
    v ...
    
    XXXX: 16 bits address (modId/tcId?)
    YY: 8 bits TC energy

  */
  
  // Create TC vectors per (roz,phi) bin
  unsigned event=0;
  std::unordered_map< unsigned, std::vector<std::pair<unsigned, HGCalTriggerCell> > > TCandEvtperBin;
  for (const auto TCs: tcPerBx) {
    for (const auto& tc: TCs) { 
      double rz = tc.rOverZ();
      rz = (rz < rzmin_ ? rzmin_ : rz);
      rz = (rz > rzmax_ ? rzmax_ : rz);
      unsigned rzbin = tc.rOverZ();//(rz_bin_size_ > 0. ? unsigned((rz - rzmin_) / rz_bin_size_) : 0);
      
      double phi = tc.phi();
      unsigned phibin = tc.phi();//(phi > stage1Config_.phiEdges().at(rzbin));

      unsigned packed_bin = 0;
      packed_bin |= ((rzbin & mask_roz_) << offset_roz_);
      packed_bin |= (phibin & mask_phi_);
      
      TCandEvtperBin[packed_bin].push_back(std::make_pair(event,tc));
    } 
    event++;
  }

  // Loop on vector per bins
  for (const auto& bin_tcs : TCandEvtperBin) {

    unsigned rzbin = ((bin_tcs.first >> offset_roz_) & mask_roz_);
    unsigned phibin = (bin_tcs.first & mask_phi_);

    std::ofstream outputStream;
    std::string fileout = outputFileName + "_Phi" + std::to_string(phibin) + "Bin" + std::to_string(rzbin) + ".txt";
    outputStream.open(fileout);
    unsigned ievt=0;
    for (const auto& evtAndTc: bin_tcs.second) {
      unsigned theEvent = evtAndTc.first;
      if(theEvent!=ievt){
	outputStream << std::endl;
	ievt++;
      }     
      HGCalTriggerCell TC = evtAndTc.second;

      unsigned TCe = TC.energy();
      unsigned TCid = (TC.index() << 6) + TC.index_cmssw(); // index_cmssw() -> TC address (in module); index() -> module hash
      unsigned TCout = (dumpAddresses) ? (TCid << offset_e_) + TCe : TCe;
      std::stringstream sstream;
      if(dumpAddresses)
	sstream << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << TCout;
      else
	sstream << TC.index() << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << TCout;
      outputStream << sstream.str() << " ";
    }
    outputStream.close();
  }
  return 0;
}


int main(int argc, char **argv) {

  // Parse input arguments 
  const std::string theTClist = argv[1];
  const std::string theCfgJSON = argv[2];
  const std::string dumpDir = argv[3];

  // Initialize tester:
  std::cout << "Initializing tester." << std::endl;
  HGCTPGEmulatorTester hgctpgTester(theCfgJSON, theTClist);

  int error_code=0; // to do

  std::cout << "Running sorting and truncation." << std::endl;
  HGCalTriggerCellsPerBx sortedTruncatedTCsPerBx;
  error_code = hgctpgTester.runAlgo(hgctpgTester.getTCs(), sortedTruncatedTCsPerBx);
  
  std::cout << "Dumping outputs." << std::endl;
  // "readable" dump
//  std::cout << "    1. before sorting and truncation" << std::endl;
//  const std::string unsortedTCdumpFile = dumpDir+"/inputTCs.txt";
//  error_code = hgctpgTester.dumpTCs(hgctpgTester.getTCs(), unsortedTCdumpFile);

//  std::cout << "    2. sorted and truncated" << std::endl;
//  const std::string sortedTCdumpFile = dumpDir+"/sortedTruncatedTCs.txt";
//  error_code = hgctpgTester.dumpTCs(sortedTruncatedTCsPerBx, sortedTCdumpFile);
  
  // Hex tables (TC address/energies)
  std::cout << "    1. before sorting and truncation - fw comparison format" << std::endl;
  const std::string unsortedTCdumpFile_fw = dumpDir+"/Inputs";
  error_code = hgctpgTester.dumpTCs_fw(hgctpgTester.getTCs(), unsortedTCdumpFile_fw, true);

  std::cout << "    2. sorted and truncated - fw comparison format" << std::endl;
  const std::string sortedTCdumpFile_fw = dumpDir+"/Results";
  error_code = hgctpgTester.dumpTCs_fw(sortedTruncatedTCsPerBx, sortedTCdumpFile_fw, true);
  
  std::cout << "Exiting." << std::endl;
  

}
