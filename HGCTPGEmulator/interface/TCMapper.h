#ifndef __L1Trigger_L1THGCal_TCMapper_h__
#define __L1Trigger_L1THGCal_TCMapper_h__

#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalStage1TruncationConfig_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalTriggerCell_SA.h"



class TCMapper {
public:

  TCMapper();
  ~TCMapper() {}

  TCMap parseTClist(std::string theInputFile) const;

  std::map< std::string, std::string > parseGeometryCfg(const std::string theCfgFile) const;

private:

};

typedef std::map<std::pair<unsigned,unsigned>, std::tuple<unsigned,unsigned,unsigned>> TCMap;

#endif
