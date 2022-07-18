import pandas as pd
import numpy as np

import load_mappings

geometry_file          = './data/triggergeom.root'
mapping_file           = './data/allocation_stage1_20200729_1.txt'
tc_uv_mapping_file     = './data/LDM_TC_Mapping.csv'
outputtcs_per_bin_file = './data/tcs_per_bin_60deg_120links_420tcs.txt'
phiedges_file          = './data/phisector_edges_210628.txt'
params                 = {'tileboard_definition' : 'V7'}

tcio , tcmap = load_mappings.extract(
    params        = params,
    geometry      = geometry_file,
    stage1mapping = mapping_file,
    tcuvmapping   = tc_uv_mapping_file,
    outputtcs     = outputtcs_per_bin_file,
    phiedges      = phiedges_file)


tcmap = tcmap[['module_hash','tcaddress','roverz','phi']]

print(tcmap)

tcmap.to_csv('./TCmap_rzphi.csv', index=False)
