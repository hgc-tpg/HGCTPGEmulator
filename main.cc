#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalHistoClusteringImpl_SA.h"
#include "L1Trigger/L1THGCal/interface/backend_emulator/HGCalHistoClusteringConfig_SA.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>

using namespace l1thgcfirmware;
int main() {

	// Fill input clusters and seeds
	std::vector< std::vector< HGCalTriggerCell> > TCs = {
		 #include "inputs.txt"
		//#include "inputs_CMSSW.txt"
	};

	// Create vector of pointers to TCs
	// Actually, this is making a copy of the inputs...fix
    std::vector< std::vector< std::shared_ptr<HGCalTriggerCell> > > TCPtrs;
	for (unsigned int iFrame = 0; iFrame < TCs.size(); ++iFrame ) {
		std::vector< std::shared_ptr<HGCalTriggerCell> > temp;
		for (unsigned int iInput = 0; iInput < TCs[iFrame].size(); ++iInput ) {
			HGCalTriggerCell tc = TCs[iFrame][iInput];
			temp.emplace_back( std::make_shared<HGCalTriggerCell>(tc.frameValid(), tc.dataValid(), tc.rOverZ(), tc.phi(), tc.layer(), tc.energy() ) );
		}
		TCPtrs.push_back(std::move(temp));
	}


	// Configuration
	ClusterAlgoConfig config;

	HGCalHistoClusteringImplSA theAlgo(config);

	// Outputs
	HGCalTriggerCellSAPtrCollection clusteredTCs_out_SA;
	HGCalTriggerCellSAPtrCollection unclusteredTCs_out_SA;
	CentroidHelperPtrCollection prioritizedMaxima_out_SA;
	CentroidHelperPtrCollection readoutFlags_out_SA;
	HGCalClusterSAPtrCollection clusterSums_out_SA;

	// Run the algorithm
	theAlgo.runAlgorithm( TCPtrs, clusteredTCs_out_SA, unclusteredTCs_out_SA, prioritizedMaxima_out_SA, readoutFlags_out_SA, clusterSums_out_SA );

	std::ofstream clusteredTCsFile;
	clusteredTCsFile.open ("clusteredTCs.txt");
	for (const auto& tc : clusteredTCs_out_SA ) {
    	clusteredTCsFile << tc->clock() << " " << tc->index() << " " << tc->rOverZ() << " " << tc->layer() << " " << tc->energy() << " " << tc->phi() << " " << tc->sortKey() << " " << tc->deltaR2() << " " << tc->dX() << " " << tc->Y() << " " << tc->frameValid() << " " << tc->dataValid() << std::endl;
	}
	clusteredTCsFile.close();

	std::ofstream unclusteredTCsFile;
	unclusteredTCsFile.open ("unclusteredTCs.txt");
	for (const auto& tc : unclusteredTCs_out_SA ) {
    	unclusteredTCsFile << tc->clock() << " " << tc->index() << " " << tc->rOverZ() << " " << tc->layer() << " " << tc->energy() << " " << tc->phi() << " " << tc->sortKey() << " " << tc->deltaR2() << " " << tc->dX() << " " << tc->Y() << " " << tc->frameValid() << " " << tc->dataValid() << std::endl;
	}
	unclusteredTCsFile.close();

	std::ofstream prioritizedMaximaFile;
	prioritizedMaximaFile.open ("prioritizedMaxima.txt");
	for (const auto& maxima : prioritizedMaxima_out_SA ) {
    	prioritizedMaximaFile << maxima->clock() << " " << maxima->index() << " " << maxima->column() << " " << maxima->row() << " " << maxima->energy() << " " << maxima->X() << " " << maxima->Y() << " " << maxima->dataValid() << std::endl;
	}
	prioritizedMaximaFile.close();

	std::ofstream readoutFlagFile;
	readoutFlagFile.open ("readoutFlags.txt");
	for (const auto& flag : readoutFlags_out_SA ) {
    	readoutFlagFile << flag->clock() << " " << flag->index() << " " << flag->column() << " " << flag->row() << " " << flag->energy() << " " << flag->X() << " " << flag->Y() << " " << flag->dataValid() << std::endl;
	}
	readoutFlagFile.close();

	std::ofstream clusterSumsFile;
	clusterSumsFile.open ("clusterSums.txt");
	for (const auto& c : clusterSums_out_SA ) {
		clusterSumsFile << c->clock() << " " << c->index() << " " << c->n_tc() << " " << c->e() << " " << c->e_em() << " " << c->e_em_core() << " " << c->e_h_early() << " " << c->w() << " " << c->n_tc_w() << " " << c->w2() << " " << c->wz() << " " << c->weta() << " " << c->wphi() << " " << c->wroz() << " " << c->weta2() << " " << c->wphi2() << " " << c->wroz2() << " " << c->layerbits() << std::endl;
	}
	clusterSumsFile.close();

}

