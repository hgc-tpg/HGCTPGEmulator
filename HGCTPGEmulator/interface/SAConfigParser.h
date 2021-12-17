#ifndef __HGCTPGEmulator_SAConfigParser_h__
#define __HGCTPGEmulator_SAConfigParser_h__

#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalTriggerCell_SA.h"

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

  SAConfigParser();
  ~SAConfigParser() {}

  std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > parseTClist(const std::string theInputFile) const;
  std::vector< std::vector <l1thgcfirmware::HGCalTriggerCell> > parseTClist(const std::string theInputFile, const std::string theTCMap) const;

  l1thgcfirmware::Stage1TruncationConfig parseCfg(const std::string theCfgFile) const;

private:
  std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > getTCmap(const std::string theTCMap) const;
};

#endif
