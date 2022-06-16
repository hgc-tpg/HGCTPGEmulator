#include "HGCTPGStage1Emulator/interface/SATCParser.h"

using namespace l1thgcfirmware;

SATCParser::SATCParser(const std::string& tcFile, const TCMap& theTCMap) :
  theTClist_(parseTClist(tcFile, theTCMap))
{}

std::vector< std::vector <HGCalTriggerCell> > SATCParser::parseTClist_s2(const std::string& theInputFile) const {
  /*
    TC reader for input format defined for Stage 2 tests
  */
  std::vector< std::vector <HGCalTriggerCell> > TCs_out;
  std::vector <HGCalTriggerCell> TCs_tmp;
  std::ifstream theFile(theInputFile);
  if (!theFile.is_open()) throw std::runtime_error("Could not open TC list.");

  std::cout << "///////////////////////" << std::endl
            << "Getting input TCs from " << theInputFile << ": " << std::endl << std::endl;
  std::string line;

  int tcid=0;
  while (std::getline(theFile,line)) {
    if(line[0]=='#' || line.empty())
      continue;
    if (line[0]=='{') { // skip lines with '{'
      continue;
    } else if (line[0]=='}') { // '}' marking end of new vector
      TCs_out.push_back(TCs_tmp);
      TCs_tmp.clear();
      tcid=0;
    } else { // actuall TC info
      // getting only hex values from line
      std::string value = line.erase(0,line.find('(')+1); // removing  "l1thgcfirmware::HGCalTriggerCell("
      value.resize(value.size()-2); //removing '), at end of line'
      value.erase(std::remove(value.begin(), value.end(), ','), value.end()); // removing comma separators

      // reading values and storing into TC object
      std::stringstream iss(value);
      std::istream_iterator<std::string> begin(iss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> tmpTCcontent(begin, end);
      HGCalTriggerCell tmpTC(((unsigned)std::stoi(tmpTCcontent.at(0).c_str(),0,16)==1),
                             ((unsigned)std::stoi(tmpTCcontent.at(1).c_str(),0,16)==1),
                             (unsigned)std::stoi(tmpTCcontent.at(2).c_str(),0,16),
                             (unsigned)std::stoi(tmpTCcontent.at(3).c_str(),0,16),
                             (unsigned)std::stoi(tmpTCcontent.at(4).c_str(),0,16),
                             (unsigned)std::stoi(tmpTCcontent.at(5).c_str(),0,16),
			     tcid);
      TCs_tmp.push_back(tmpTC);
      tmpTC.setIndex(tcid);
      tcid++;
      tmpTCcontent.clear();
    }
  }
  return TCs_out;
}

std::vector< std::vector <HGCalTriggerCell> > SATCParser::parseTClist(const std::string& theInputFile, const TCMap& theTCMap) const {
  /*
    TC reader for input format defined for Stage 1 tests
  */

  std::cout<<"---- parsing tc list"<<std::endl;

  std::vector< std::vector <HGCalTriggerCell> > TCs_out;
  std::vector <HGCalTriggerCell> TCs_tmp;


  // Get list of TCs to be processed
  std::ifstream theFile(theInputFile);
  if (!theFile.is_open()) throw std::runtime_error("Could not open Mod_stimuli list.");

  json theTCjson;
  theFile >> theTCjson;
  int evt=0;
  // loop on events
  for (auto& event: theTCjson.at("event")) {

    std::cout << "Event " << evt << ":" << std::endl;

    // loop on modules
    unsigned moduleId = 0;
    for (json::iterator moduleIt = event.at("module").begin(); moduleIt != event.at("module").end(); ++moduleIt) {

      // store module hash
      unsigned moduleHash = atoi(moduleIt.key().c_str());
      json moduleTcs = event.at("module")[moduleIt.key()];

      if(moduleHash<10000) { std::cout << "Mod" << moduleHash << ":  |"; }
      else {std::cout << "Mod" << moduleHash << ": |";}
      // loop on TCs in module
      unsigned tcid = 0;
      for (const std::string& tc: moduleTcs.at("tc")) {

	// get rz/phi values from map
	std::pair<unsigned,unsigned> modHash_tcid{moduleHash,tcid};
	auto roz_phi= theTCMap.find(modHash_tcid);
	

	// fill TC object
	if (roz_phi!=theTCMap.end()) { // check if mapping entry exists

	  HGCalTriggerCell tmpTC(true, // not important for S1?
				 true, // not important for S1?
				 std::get<0>(roz_phi->second), // roverz (w/ magic numbers)
				 std::get<1>(roz_phi->second), // phi (w/ magic numbers)
				 50, // layer - dummy for now, to correct (although not used for S1)
				 (unsigned)std::stoi(tc.c_str(),0,16), // energy
				 tcid); // id_cmssw (FIXME)
	  
	  // set "firmware-style" module ID"
	  tmpTC.setIndex(moduleId);
	  // fill event's TC vector
	  TCs_tmp.push_back(tmpTC);

	  // printout for zero energy cell
	  if(0==(unsigned)std::stoi(tc.c_str(),0,16)) {std::cout << "--|";}
	  // printout for non-zero enegry cell
	  else {std::cout << tc.c_str() << "|";}

	} else { 
          // no mapping info found
	  std::cout <<"__|";
	}

	tcid++;

      }
      std::cout << std::endl;
      moduleId++;
    }

    std::cout << std::endl;
    evt++;
    TCs_out.push_back(TCs_tmp);
    TCs_tmp.clear();

  }

  return TCs_out;

}
