#ifndef __HGCTPGEmulator_SAConfigParser_h__
#define __HGCTPGEmulator_SAConfigParser_h__

#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalTriggerCell_SA.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <stdio.h>
#include <bits/stdc++.h>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

class SAConfigParser {
public:

  SAConfigParser(const std::string& inputFile);
  ~SAConfigParser() {}

  // getters
  const l1thgcfirmware::Stage1TruncationConfig& getCfg() const {return theCfg_;}
  const std::map< std::pair<unsigned,unsigned>, std::pair<unsigned,unsigned> >& getTCmap() const {return theTCMap_;}

private:
  int parseCfg(const std::string& theCfgFile, l1thgcfirmware::Stage1TruncationConfig& theConfig) const;
  int setTCmap(const std::string& theTCMapName, 
	       std::map< std::pair<unsigned,unsigned>, std::pair<unsigned,unsigned> >& TCmap_out) const;

  l1thgcfirmware::Stage1TruncationConfig theCfg_;
  std::map< std::pair<unsigned,unsigned>, std::pair<unsigned,unsigned> > theTCMap_;

  double rotatedphi(double phi) const;

};

#endif
