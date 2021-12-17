
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
    phisector_edges_file = kwargs['phiedges']
    tileboard_definition = params['tileboard_definition']
    #filters = 'fpga=={fpga} & roverzbin<{roz}'.format(fpga=params['fpga'], roz=params['roz_max'])
    #print(filters)
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
    if tileboard_definition=='V7':
        sector0_df[['tileboard_ieta', 'tileboard_iphi']] = sector0_df.apply(tileboard_number_V7, axis=1)
    elif tileboard_definition=='TpgV7':
        sector0_df[['tileboard_ieta', 'tileboard_iphi']] = sector0_df.apply(tileboard_number_TpgV7, axis=1)
    else:
        raise RuntimeError('Unkown tileboard definition '+tileboard_definition)
    sector0_df[['triggercelle', 'triggercellp']] = sector0_df.apply(sci_tc_etaphi, axis=1)
    sector0_df.loc[sector0_df.subdet>=2, 'layer'] += 28
    sector0_df['subdet'] = sector0_df['subdet'].replace([1,2], 0)

    sector0_df['subdet'] = sector0_df['subdet'].replace(10, 1)
    sector0_df['roverz'] = np.sqrt(sector0_df.x**2+sector0_df.y**2)/sector0_df.z
    minroz = np.min(sector0_df.roverz)
    maxroz = np.max(sector0_df.roverz)
    sizeroz = maxroz-minroz
    binsize = (sizeroz*1.001)/42
    print('minroz={0}, maxroz={1}'.format(minroz, maxroz))
    print('sizeroz=', sizeroz)
    print('binsize=', binsize)
    sector0_df['roverzbin'] = ((sector0_df.roverz-minroz)/binsize).astype(dtype=int)
    print('sector0_df roverzbin max=', sector0_df.roverzbin.max())
    sector0_df['phi'] = np.arctan2(sector0_df.y, -sector0_df.x) # Use -x such that the sector 0 is between [0,120deg]
    #sector0_df['phisector'] = np.where(sector0_df.phi*180/np.pi>-30, 1, 0)
    def phisector(row, edges):
        rozbin = int(row.roverzbin)
        sector = (row.phi>edges[rozbin])[0]
        return 1 if sector else 0
    phisector_edges = pd.read_csv(phisector_edges_file, header=None, sep=' ')
    sector0_df['phisector'] = sector0_df.apply(lambda row : phisector(row, phisector_edges), axis=1)
    print('>> Computing modules hash')
    mapping['module_hash'] = mapping.apply(module_hash, axis=1)
    sector0_df['module_hash'] = sector0_df.apply(module_hash_cmssw, axis=1)
    mapping.set_index('module_hash', inplace=True)
    print('>> Mapping TCs to FPGAs')
    sector0_df['fpga'] = sector0_df.apply(lambda row: board(row,mapping), axis=1).astype(dtype=int)
    #  tcs_fpga = sector0_df.query(filters)
    tcs_fpga = sector0_df
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

    print('tcs_fpga roverzbin max=', tcs_fpga.roverzbin.max())
    temp_output = tcs_fpga\
    .reset_index()[['fpga', 'phisector', 'roverzbin', 'module_hash', 'tcaddress']]\
    .groupby(['fpga', 'phisector', 'roverzbin', 'module_hash'])\
    .apply(list_tcs).reset_index()\
    .rename(columns={'module_hash':'hash'})\
    .groupby(['fpga', 'phisector', 'roverzbin'])\
    .apply(list_modules).reset_index()

    print('temp_output roverzbin max=', temp_output.roverzbin.max())
    outputtcs_per_bin = pd.read_csv(outputtcs_per_bin_file, header=None)
    import math as m
    def apply_ntcout(row):
        # ntcout = int(m.ceil(outputtcs_per_bin[row.roverzbin]/2))
        ntcout = int(outputtcs_per_bin[row.roverzbin])
        return max(ntcout, 1)

    temp_output['ntcout'] = temp_output.apply(apply_ntcout, axis=1)
    print('temp_output roverzbin max=', temp_output.roverzbin.max())
    full_output = temp_output.rename(columns={'phisector':'phi',
                                'roverzbin':'roz',
                               })
    print('full_output roverzbin max=', full_output.roz.max())

    return full_output, tcs_fpga

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
        board = 0
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

def tileboard_number_V7(row):
# https://github.com/snwebb/hgcal-linkmapping/blob/f9d90635faa5a514d03b895dfd29c899d065d43f/fluctuation.py#L139-L149
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


def tileboard_number_TpgV7(row):
# https://github.com/snwebb/hgcal-linkmapping/blob/f9d90635faa5a514d03b895dfd29c899d065d43f/fluctuation.py#L150-L157
    subdet = row.subdet
    layer = row.layer
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

    split = 12
    if layer > 40:
        split = 8
    if ( eta <= split ):
        ep = 0
    else:
        ep = 1
    print(ep,pp)
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
