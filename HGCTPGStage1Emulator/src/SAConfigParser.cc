#include "HGCTPGStage1Emulator/interface/SAConfigParser.h"

using namespace l1thgcfirmware;

SAConfigParser::SAConfigParser(const std::string& inputFile){
  parseCfg(inputFile, theCfg_);
  setTCmap(inputFile, theTCMap_);
}

int SAConfigParser::setTCmap(const std::string& theTCMap, 
			     std::map< std::pair<unsigned,unsigned>, std::pair<unsigned,unsigned> >& TCmap_out) const {
/*
  TCmap reader from LLR S1 firmware generator configuration
*/
  json theJSON;

  // Get list of TCs to be processed
  std::ifstream theFile(theTCMap);
  if (!theFile.is_open()) throw std::runtime_error("Could not open Mod_stimuli list.");

  theFile >> theJSON;
  json theCfgJSON = theJSON["TriggerCellMap"];

  // loop on events
  for (auto& module: theCfgJSON["Module"]) {

    unsigned moduleHash = std::stoul((std::string)module.at("hash"));
    
    for (auto &tc: module["tcs"]) {

      unsigned tcRoz = (unsigned)tc["roz_bin"];
      unsigned tcPhi = (unsigned)tc["phi_bin"];

      TCmap_out.insert({std::pair<unsigned,unsigned>(moduleHash, tc["tcid"]),
	                std::pair<unsigned,unsigned>(tcRoz, tcPhi)});
    }
  }

  return 1;
}

int SAConfigParser::parseCfg(const std::string& theCfgFile, Stage1TruncationConfig& theConfig) const {
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
  json theCfgJSON;
  theJSONCfg >> theCfgJSON;
  
  json theCfg = theCfgJSON["TruncationConfig"];

  // - doTruncation (bool)
  bool doTruncation = true;
  /*
  try {
    doTruncation = theCfg.at("doTruncation");
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: doTruncation");
  }
  */

  // - rozMin/rozMax (double)
  double rozMin = 0.;
  double rozMax = 0.;
  try {
    rozMin = (double)theCfg.at("rozMin") * rozphi_scale_;
    rozMax = (double)theCfg.at("rozMax") * rozphi_scale_;
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
    for (const unsigned& theBinMaxTCs: theCfg.at("maxTcsPerBin"))
      maxTCsPerBin.push_back(theBinMaxTCs);
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: maxTCsPerBin");
  }

  // - phiSectorEdges (vector<double>) from an array
  std::vector<double> phiSectorEdges;
  try {
    for (const double& theEdge: theCfg.at("phiSectorEdges"))
      phiSectorEdges.push_back(theEdge * rozphi_scale_);
  } catch (const json::exception& e) {
    throw std::runtime_error("Could not read required parameter: phiSectorEdge");
  }

  // Fill in CMSSW-format config
  theConfig.setParameters(doTruncation, rozMin, rozMax, rozBins, maxTCsPerBin, phiSectorEdges);

  return 1;

}

double SAConfigParser::rotatedphi(double phi) const {

  if (phi > 2.*M_PI / 3. and phi < 4.* M_PI / 3.)
    phi = phi - (2. * M_PI / 3.);
  else if (phi < -2.*M_PI / 3. and phi > -4.* M_PI / 3.)
    phi = 2.*M_PI - phi - (2. * M_PI / 3.);
  else if (phi > -2.*M_PI / 3. and phi < 0)
    phi = 2.*M_PI - phi + (2. * M_PI / 3.);
  else if (phi > 4. * M_PI / 3. and phi < 2 * M_PI)
    phi = phi - (4. * M_PI / 3.);

  return phi;
}
