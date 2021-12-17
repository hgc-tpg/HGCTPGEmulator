#include "HGCTPGEmulator/interface/SAConfigParser.h"

using namespace l1thgcfirmware;

SAConfigParser::SAConfigParser() {}

std::vector< std::vector <HGCalTriggerCell> > SAConfigParser::parseTClist(const std::string theInputFile) const {
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
  while (std::getline(theFile,line)) {
    if(line[0]=='#' || line.empty())
      continue;
    if (line[0]=='{') { // skip lines with '{'
      continue;
    } else if (line[0]=='}') { // '}' marking end of new vector
      TCs_out.push_back(TCs_tmp);
      TCs_tmp.clear();
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
                             (unsigned)std::stoi(tmpTCcontent.at(5).c_str(),0,16));
      TCs_tmp.push_back(tmpTC);
      tmpTCcontent.clear();
    }
  }
  return TCs_out;
}

std::vector< std::vector <HGCalTriggerCell> > SAConfigParser::parseTClist(const std::string theInputFile, const std::string theTCMap) const {
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

  std::string tc_line;

  std::cout << "looping on tc input file lines" << std::endl;
  bool firstline=true;
  while (std::getline(theFile,tc_line)) {
    /* Here each line should be a string of 48 space-separated hex values corresponding to the TC energies. */

    if(tc_line[0]=='#' || tc_line.empty()) continue; // skip commented/empty lines

    if(tc_line[0]!='E'){ // "Module" line: fill event's TC vector

      // split line into "module hash" and "TC energies":
      unsigned module_hash = 0;
      std::string line_bit;
      std::stringstream tcline(tc_line);

      while (std::getline(tcline, line_bit, ':')) {

        if(line_bit[0]=='M') { //the bit is 'ModuleXXXX '

          // extract module hash
          std::string value = line_bit.erase(0,line_bit.find('e')+1); // removing  "Module"
          value.resize(value.size()-1); // removing trailing space
          module_hash = atoi(value.c_str()); // save hash as unsigned

        } else { // the bit is the list of TC energies

          // getting only hex values from line
          std::string value = line_bit.erase(0,1); // remove space at start
          std::stringstream iss(line_bit);
          std::istream_iterator<std::string> begin(iss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> tmpTCcontent(begin, end); // store TC energy list in string vector

          // loop on TC vector and fill TC objects
          unsigned tcid = 0;
          std::cout << "mod"<<module_hash<<" : ";
          for (const auto& value: tmpTCcontent) {

            // get rz/phi values from map
            std::pair<unsigned,unsigned> modHash_tcid{module_hash,tcid};
            auto roz_phi= TCmap_out.find(modHash_tcid);

            // fill TC object
            if (roz_phi!=TCmap_out.end()) { // check if mapping entry exists
              HGCalTriggerCell tmpTC(true, //not important for S1?
                                     true, //not important for S1?
                                     (4096/0.7)*std::get<0>(roz_phi->second), //roverz (w/ magic numbers)
                                     (4096/0.7)*std::get<1>(roz_phi->second), //phi (w/ magic numbers)
                                     50, //layer - dummy for now, to correct (although not used for S1)
                                     (unsigned)std::stoi(value.c_str(),0,16)); // energy

              // fill event's TC vector
              TCs_tmp.push_back(tmpTC);

              std::cout << tmpTC.energy()<< ",";
              tcid++;

            } else { // no mapping info found

              std::cout <<"X,";
              tcid++;

            } // end if-else 'check if mapping entry exists'
          } // end loop on TCs

          std::cout << std::endl;

        } //end if-else 'line_bit'
      } //end loop 'line_bit'

    } else { // "Event line": store previous event's TC vector

      std::cout<<tc_line<<std::endl;

      if(firstline){ // event's TC vector is empty, do nothing

        firstline=false;

      } else { // store previous event's TC vector

        TCs_out.push_back(TCs_tmp);
        TCs_tmp.clear();

      }
    }
  }

  // store last event's TC vector
  TCs_out.push_back(TCs_tmp);
  TCs_tmp.clear();

  return TCs_out;
}

std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > SAConfigParser::getTCmap(const std::string theTCMap) const {
/*
  TCmap reader from LLR S1 firmware generator configuration
*/
std::map< std::pair<unsigned,unsigned>, std::pair<double,double> > TCmap_out;
  std::ifstream TCMap(theTCMap);
  if (!TCMap.is_open()) throw std::runtime_error("Could not open TC map.");
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

Stage1TruncationConfig SAConfigParser::parseCfg(const std::string theCfgFile) const {
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
  //cfg param:
  bool doTruncation;
  double rozMin, rozMax;
  unsigned rozBins;
  std::vector<unsigned> maxTCsPerBin;
  std::vector<double> phiSectorEdges;
  std::ifstream theCfg(theCfgFile);
  if (!theCfg.is_open()) throw std::runtime_error("Could not open config file.");

  std::string line;
  while (std::getline(theCfg,line)) {
    if(line[0]=='#' || line.empty())
 	    continue;
    auto delimPos = line.find("=");
    std::string name = line.substr(0,delimPos);
    std::string value = line.substr(delimPos+1);
    // bool
    if (name == "doTruncation")
      doTruncation = (value=="1");
    //single values
    if (name == "rozMin")
      rozMin = (double)atof(value.c_str());
    if (name == "rozMax")
      rozMax = (double)atof(value.c_str());
    if (name == "rozBins")
      rozBins = (unsigned)atof(value.c_str());
    // vectors
    if (name == "maxTCsPerBin"){
      std::stringstream iss(value);
      unsigned number;
      while ( iss >> number ) {
        maxTCsPerBin.push_back(number);
      }
    }
    if (name == "phiSectorEdges") {
      std::stringstream iss(value);
      double number;
      while (iss >> number) {
        phiSectorEdges.push_back(number);
      }
    }
  }
  Stage1TruncationConfig theConfig(doTruncation, rozMin, rozMax, rozBins, maxTCsPerBin, phiSectorEdges);
  return theConfig;
}
