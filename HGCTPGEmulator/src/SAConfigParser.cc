#include "HGCTPGEmulator/interface/SAConfigParser.h"

using namespace l1thgcfirmware;

SAConfigParser::SAConfigParser() {}

std::vector< std::vector <HGCalTriggerCell> > SAConfigParser::parseTClist(const std::string& theInputFile) const {
  /*
    TC reader for input format defined by Emyr
    Mostly there to avoid the hard coded "include" of the TC list, passing it instead as an argument to the exe
    The parser could probably be cleaner, but it is fully functional
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

std::vector< std::vector <HGCalTriggerCell> > SAConfigParser::parseTClist(const std::string& theInputFile, const std::string& theTCMap) const {
  /*
    TC reader for 'Mod_stimuli' files
  */
  std::vector< std::vector <HGCalTriggerCell> > TCs_out;
  std::vector <HGCalTriggerCell> TCs_tmp;

  // Reading TC map from csv
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > TCmap_out = SAConfigParser::getTCmap(theTCMap);

  // Get list of TCs to be processed
  std::ifstream theFile(theInputFile);
  if (!theFile.is_open()) throw std::runtime_error("Could not open Mod_stimuli list.");

  json theTCjson;
  theFile >> theTCjson;
  int evt=0;
  std::cout << "Event " << evt << ":" << std::endl;
  // loop on events
  for (auto& event: theTCjson.at("event")) {

    std::cout << "Event " << evt << ":" << std::endl;

    // loop on modules
    for (json::iterator moduleIt = event.at("module").begin(); moduleIt != event.at("module").end(); ++moduleIt) {

      // store module hash
      unsigned moduleHash = atoi(moduleIt.key().c_str());
      json moduleTcs = event.at("module")[moduleIt.key()];

      std::cout << "Module " << moduleHash << ": ";

      // loop on TCs in module
      unsigned tcid = 0;
      for (const std::string& tc: moduleTcs.at("tc")) {

	// get rz/phi values from map
	std::pair<unsigned,unsigned> modHash_tcid{moduleHash,tcid};
	auto roz_phi= TCmap_out.find(modHash_tcid);

	// fill TC object
	if (roz_phi!=TCmap_out.end()) { // check if mapping entry exists

	  HGCalTriggerCell tmpTC(true, //not important for S1?
				 true, //not important for S1?
				 (4096/0.7)*std::get<0>(roz_phi->second), //roverz (w/ magic numbers)
				 (4096/0.7)*std::get<1>(roz_phi->second), //phi (w/ magic numbers)
				 50, //layer - dummy for now, to correct (although not used for S1)
				 (unsigned)std::stoi(tc.c_str(),0,16), // energy
				 tcid); // id_cmssw
	  tmpTC.setIndex(tcid);
	  // fill event's TC vector
	  TCs_tmp.push_back(tmpTC);

	  std::cout << tmpTC.energy()<< ",";
	  tcid++;

	} else { // no mapping info found

	  std::cout <<"X,";
	  tcid++;

	}

      }
      std::cout << std::endl;
    }

    std::cout << std::endl;
    evt++;
    TCs_out.push_back(TCs_tmp);
    TCs_tmp.clear();

  }

  return TCs_out;

}

std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > SAConfigParser::getTCmap(const std::string& theTCMap) const {
/*
  TCmap reader from LLR S1 firmware generator configuration
*/
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > TCmap_out;
  std::ifstream TCMap(theTCMap);
  if (!TCMap.is_open()) throw std::runtime_error("Could not open TC map.");
  std::cout<<theTCMap<<std::endl;

  std::string tc, colname;
  std::string tcline;
  double val;
  std::tuple<unsigned,unsigned,double,double,unsigned> tcvals;
  while (std::getline(TCMap,tcline)) {
    std::stringstream ss(tcline);
    unsigned colIdx = 0;
    while (ss >> val) {
      if (colIdx==0) { // module_hash
        std::get<0>(tcvals) = (unsigned)val;
      } else if (colIdx==1) { // tcaddress
        std::get<1>(tcvals) = (unsigned)val;
      } else if (colIdx==2) { // roverz
        std::get<2>(tcvals) = val;
      } else if (colIdx==3) { // phi
        std::get<3>(tcvals) = val;
      }
      if(ss.peek() == ',') ss.ignore();
      colIdx++;
    }
    TCmap_out.insert({std::pair<unsigned,unsigned>(std::get<0>(tcvals),
                                         std::get<1>(tcvals)),
                      std::pair<double,double>(std::get<2>(tcvals),
                                               std::get<3>(tcvals))});
  }
  return TCmap_out;
}

Stage1TruncationConfig SAConfigParser::parseCfg(const std::string& theCfgFile) const {
  /*
    Parser for Stage 1 configuration
    Currently handling a fixed set of parameters:
    doTruncation=[true,false]
    rozMin=Value (double)
    rozMax=Value (double)
    rozBins=Value (unsigned)
    maxTCsPerBin={Value1,Value2,...} (std::vector<unsigned>)
    phiSectorEdges={Value1,Value2,...} (std::vector<double>)
  */

  // Read json file into string
  std::ifstream theJSONCfg(theCfgFile);
  if (!theJSONCfg.is_open()) throw std::runtime_error("Could not open config file.");

  // Parse string into rapidjson Document
  json theCfg;
  theJSONCfg >> theCfg;

  // - doTruncation (bool)
  bool doTruncation = false;
  try {
    doTruncation = theCfg.at("doTruncation");
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: doTruncation");
  }


  // - rozMin/rozMax (double)
  double rozMin = 0.;
  double rozMax = 0.;
  try {
    rozMin = theCfg.at("rozMin");
    rozMax = theCfg.at("rozMax");
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: rozMin,rozMax");
  }

  // - rozBins (unsigned)
  unsigned rozBins = 0;
  try {
    rozBins = theCfg.at("rozBins");
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: rozBins");
  }

  // - maxTCsPerBin (vector<unsigned>) from an array
  std::vector<unsigned> maxTCsPerBin;
  try {
    for (const unsigned& theBinMaxTCs: theCfg.at("maxTCsPerBin"))
      maxTCsPerBin.push_back(theBinMaxTCs);
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: maxTCsPerBin");
  }

  // - phiSectorEdges (vector<double>) from an array
  std::vector<double> phiSectorEdges;
  try {
    for (const double& theEdge: theCfg.at("phiSectorEdges"))
      phiSectorEdges.push_back(theEdge);
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: phiSectorEdge");
  }

  // Fill in CMSSW-format config
  Stage1TruncationConfig theConfig(doTruncation, rozMin, rozMax, rozBins, maxTCsPerBin, phiSectorEdges);
  return theConfig;

}
