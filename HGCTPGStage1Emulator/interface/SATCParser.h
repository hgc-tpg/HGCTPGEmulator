#ifndef __HGCTPGEmulator_SATCParser_h__
#define __HGCTPGEmulator_SATCParser_h__

#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalTriggerCell_SA.h"

#include "HGCTPGStage1Emulator/interface/SAConfigParser.h"

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

typedef std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > TCMap;

class SATCParser {

public:

  SATCParser(const std::string& tcFile, const TCMap& theTCMap);
  ~SATCParser() {}


  std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > TClist() const {return theTClist_;}
  const double magic_number(){return rozphi_scale_;}

private:
  std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > parseTClist_s2(const std::string& theInputFile) const;
  std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > parseTClist(const std::string& theInputFile, const TCMap& theTCMap) const;

  const std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > theTClist_;
  //TCMap theTCMap_;

  // magic number
  static constexpr double rozphi_scale_ = 4096./0.7;

};


#endif
