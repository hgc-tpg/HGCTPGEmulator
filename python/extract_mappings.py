#!/usr/bin/env python3

import os
import optparse
import yaml
import pickle
import numpy as np
import pandas as pd
import uproot

def extract(**kwargs):
    print(kwargs)
    params = kwargs['params']
    geometry_file = kwargs['geometry']
    mapping_file = kwargs['stage1mapping']
    tc_uv_mapping_file = kwargs['tcuvmapping']
    outputtcs_per_bin_file = kwargs['outputtcs']
    filters = 'fpga=={fpga} & roverzbin<{roz}'.format(fpga=params['fpga'], roz=params['roz_max'])
    print(filters)
    print('>> Reading CMSSW geometry')
    geometry = uproot.open(geometry_file)['hgcaltriggergeomtester/TreeTriggerCells']
    geometry_df = geometry.arrays(['zside', 'subdet', 'layer', 'waferu', 'waferv',
                                  'triggercellu', 'triggercellv', 'triggercellieta', 'triggercelliphi', 
                                 'x', 'y', 'z'], library='pd')
    geometry_df = geometry_df[geometry_df.zside>0]
    print('>> Reading Stage 1 mapping')
    mapping = read_stage1_mapping(mapping_file).astype(dtype=int)
    mapping = mapping[np.logical_not(mapping[['scintillator', 'layer', 'ueta', 'vphi']].duplicated(keep='first'))]
    print('>> Extracting sector0')
    geometry_df['sector'] = geometry_df.apply(sector, axis=1)
    sector0_df = geometry_df[geometry_df.sector==0]
    print('>> Retrieving bins')
    sector0_df[['tileboard_ieta', 'tileboard_iphi']] = sector0_df.apply(tileboard_number, axis=1)
    sector0_df[['triggercelle', 'triggercellp']] = sector0_df.apply(sci_tc_etaphi, axis=1)
    sector0_df.loc[sector0_df.subdet>=2, 'layer'] += 28
    sector0_df['subdet'] = sector0_df['subdet'].replace([1,2], 0)

    sector0_df['subdet'] = sector0_df['subdet'].replace(10, 1)
    sector0_df['roverz'] = np.sqrt(sector0_df.x**2+sector0_df.y**2)/sector0_df.z
    minroz = np.min(sector0_df.roverz)
    maxroz = np.max(sector0_df.roverz)
    sizeroz = maxroz-minroz
    binsize = (sizeroz*1.001)/42
    sector0_df['roverzbin'] = ((sector0_df.roverz-minroz)/binsize).astype(dtype=int)
    sector0_df['phi'] = np.arctan2(sector0_df.x, sector0_df.y)
    sector0_df['phisector'] = np.where(sector0_df.phi*180/np.pi>-30, 1, 0)
    print('>> Computing modules hash')
    mapping['module_hash'] = mapping.apply(module_hash, axis=1)
    sector0_df['module_hash'] = sector0_df.apply(module_hash_cmssw, axis=1)
    mapping.set_index('module_hash', inplace=True)
    print('>> Mapping TCs to FPGAs')
    sector0_df['fpga'] = sector0_df.apply(lambda row: board(row,mapping), axis=1).astype(dtype=int)
    tcs_fpga = sector0_df.query(filters)
    print('>> Extracting TC addresses')
    uv_mapping = pd.read_csv(tc_uv_mapping_file).set_index(['TC_U', 'TC_V']).sort_index()
    tcs_fpga['tcaddress'] = tcs_fpga.apply(lambda row: tc_address(row, uv_mapping), axis=1)
    print('>> Preparing output')
    def list_tcs(group):
        out = pd.Series([len(group.tcaddress), list(group.tcaddress)], index =['ntc', 'tcs'])
        return out 

    def list_modules(group):
        module = group[['hash', 'ntc', 'tcs']].to_dict('records')
        out = pd.Series([len(group.hash),
                         sum(group.ntc),
                         module],
                        index =['nmod', 'ntcin', 'mods'])
        return out

    temp_output = tcs_fpga\
    .reset_index()[['phisector', 'roverzbin', 'module_hash', 'tcaddress']]\
    .groupby(['phisector', 'roverzbin', 'module_hash'])\
    .apply(list_tcs).reset_index()\
    .rename(columns={'module_hash':'hash'})\
    .groupby(['phisector', 'roverzbin'])\
    .apply(list_modules).reset_index()

    outputtcs_per_bin = pd.read_csv(outputtcs_per_bin_file, header=None)
    import math as m
    def apply_ntcout(row):
        print("WARNING: the number of output TCS per bin is divided by 2")
        ntcout = int(m.ceil(outputtcs_per_bin[row.roverzbin]/2))
        return max(ntcout, 1)

    temp_output['ntcout'] = temp_output.apply(apply_ntcout, axis=1)
    full_output = temp_output.rename(columns={'phisector':'phi',
                                'roverzbin':'roz',
                               }).to_dict('records')

    output = {'bins':full_output}
    return {'bins':full_output}


def extract_lpgbt_modules(tokens, board):
    module_list = []
    lpgbt = int(tokens[0])
    nmodules = int(tokens[1])
    if len(tokens)!=2+nmodules*5:
        raise RuntimeError('Ill-formed line in mapping file ("'+' '.join(tokens)+"'")
    for module in range(nmodules):
        index = 2+module*5
        scintillator = int(tokens[index])
        layer = int(tokens[index+1])
        ueta = int(tokens[index+2])
        vphi = int(tokens[index+3])
        elinks = int(tokens[index+4])
        module_list.append([lpgbt, board, scintillator, layer, ueta, vphi, elinks])
    return pd.DataFrame(module_list, columns=['lpgbt', 'board', 'scintillator', 'layer', 'ueta', 'vphi', 'elinks'])


def read_stage1_mapping(filename):
    mapping = pd.DataFrame(columns=['lpgbt', 'board', 'scintillator', 'layer', 'ueta', 'vphi', 'elinks'])
    with open(filename) as f:
        lines = f.readlines()
        board_number = 0
        for line in lines[1:]:
            line = line.strip()
            tokens = line.split(' ')
            if len(tokens)==1:
                board = int(tokens[0])
            else:
                df = extract_lpgbt_modules(tokens, board)
                mapping = mapping.append(df)
    return mapping

def sector(row):
    subdet = int(row.subdet)
    sector = 0
    if subdet==10:
        eta = int(row.triggercellieta)
        phi = int(row.triggercelliphi)
        if phi > 24 and phi <= 72:
            sector = 0
        elif phi > 72 and phi <= 120:
            sector = 2
        else:
            sector = 1
    else:
        u = int(row.waferu)
        v = int(row.waferv)
        layer = int(row.layer)

        if subdet==1: # CE-E    
            if u>0 and v>=0:
                sector = 0
            elif u>=v and v<0:
                sector = 2
            else:
                sector = 1

        elif (layer%2)==1: # CE-H Odd
            if u>=0 and v>=0:
                sector = 0
            elif u>v and v<0:
                sector=2
            else:
                sector=1

        else: # CE-H Even
            if u>=1 and v>=1:
                sector = 0
            elif u>=v and v<1:
                sector=2
            else:
                sector=1
    return int(sector)

def tileboard_number(row):
    subdet = row.subdet
    if subdet<10:
        return pd.Series([0, 0], dtype=int)
    eta = row.triggercellieta
    phi = row.triggercelliphi
    sector = 0
    if phi > 24 and phi <= 72:
        sector = 0
    elif phi > 72 and phi <= 120:
        sector = 2
    else:
        sector = 1

    ep = 0
    pp = 0

    if sector==0:
        pp = phi-24
    elif sector==2:
        pp = phi-72
    elif sector==1:
        if phi<=24:
            phi += 144
        pp = phi-120

    pp = (pp-1)//4 #Phi index 1-12

    if eta <= 3:
        ep = 0
    elif eta <= 9:
        ep = 1
    elif eta <= 13:
        ep = 2
    elif eta <= 17:
        ep = 3
    else:
        ep = 4
   
    return pd.Series([ep, pp], dtype=int)

def sci_tc_etaphi(row):
    subdet = row.subdet
    if subdet<10:
        return pd.Series([0, 0], dtype=int)
    eta = row.triggercellieta
    phi = row.triggercelliphi
    sector = 0
    if phi > 24 and phi <= 72:
        sector = 0
    elif phi > 72 and phi <= 120:
        sector = 2
    else:
        sector = 1

    ep = 0
    pp = 0

    if sector==0:
        pp = phi-24
    elif sector==2:
        pp = phi-72
    elif sector==1:
        if phi<=24:
            phi += 144
        pp = phi-120

    pp = (pp-1)%4 #Phi inside tileboard

    if eta <= 3:
        ep = eta
    elif eta <= 9:
        ep = eta-4
    elif eta <= 13:
        ep = eta-10
    elif eta <= 17:
        ep = eta-14
    else:
        ep = eta-18
   
    return pd.Series([ep, pp], dtype=int)

def module_hash(row):
    return (row.scintillator<<14) + (row.layer<<8) + (row.ueta<<4) + row.vphi

def module_hash_cmssw(row):
    subdet = int(row.subdet)
    layer = int(row.layer)
    ueta = int(row.waferu) if subdet==0 else int(row.tileboard_ieta)
    vphi = int(row.waferv) if subdet==0 else int(row.tileboard_iphi)
    return (subdet<<14) + (layer<<8) + (ueta<<4) + vphi

def board(row, mapping):
    try:
        board = mapping.loc[row.module_hash]['board']
    except KeyError:
        board = -1
    return board

def tc_address(row, uv_mapping):
    if row.subdet==1:
        address = (int(row.triggercelle)<<2) + int(row.triggercellp)
    else:
        try:
            address = uv_mapping.loc[(row.triggercellu, row.triggercellv)]['ECON_TC_Number_PostMux']
        except KeyError:
            address = -1
    return address





def main(opt):
    params = None
    with open(opt.cfg_file, 'r') as cfg_file:
        params = yaml.safe_load(cfg_file)
    print(params)
    data = extract(\
            geometry=opt.geom_file,
            stage1mapping=opt.s1map_file,
            tcuvmapping=opt.tcaddr_file,
            outputtcs=opt.tcout_file,
                params=params)
    pickle.dump(data, open(opt.out_file, 'wb'))



if __name__=='__main__':
    parser = optparse.OptionParser()
    parser.add_option("--cfg",type="string", dest="cfg_file", help="select the configuration file")
    parser.add_option("--geo",type="string", dest="geom_file", help="select the geometry file")
    parser.add_option("--s1map",type="string", dest="s1map_file", help="select the stage-1 mapping file")
    parser.add_option("--tcaddr",type="string", dest="tcaddr_file", help="select the TC addresses file")
    parser.add_option("--tcout",type="string", dest="tcout_file", help="select the output TCs per bin file")
    parser.add_option("--out",type="string", dest="out_file", help="select the output file")
    (opt, args) = parser.parse_args()
    main(opt)

