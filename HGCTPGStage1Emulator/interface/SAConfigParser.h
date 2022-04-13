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
  l1thgcfirmware::Stage1TruncationConfig getCfg() const {return theCfg_;}
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > getTCmap() const {return theTCMap_;}
  const double magic_number(){return rozphi_scale_;}

private:
  l1thgcfirmware::Stage1TruncationConfig parseCfg(const std::string& theCfgFile) const;
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > setTCmap(const std::string& theTCMap) const;

  l1thgcfirmware::Stage1TruncationConfig theCfg_;
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > theTCMap_;

  double rotatedphi(double phi) const;

  // magic number
  static constexpr double rozphi_scale_ = 4096./0.7;

};

#endif
