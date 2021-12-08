#!/bin/bash
g++  -std=c++14 -I./ -I../  main.cc L1Trigger/L1THGCal/src/backend_emulator/HGCalHistoClusteringImpl_SA.cc L1Trigger/L1THGCal/src/backend_emulator/HGCalHistoClusteringConfig_SA.cc L1Trigger/L1THGCal/src/backend_emulator/DistServer.cc  L1Trigger/L1THGCal/src/backend_emulator/HGCalHistogramCell_SA.cc L1Trigger/L1THGCal/src/backend_emulator/HGCalCluster_SA.cc
