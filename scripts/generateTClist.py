import numpy as np

nevts = 10

mod_stimuli_files = {

    27409 : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod27409_stimuli.txt",
    27927 : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod27927_stimuli.txt",
    9521  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9521_stimuli.txt",
    9522  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9522_stimuli.txt",
    9538  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9538_stimuli.txt",
    9539  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9539_stimuli.txt",
    9555  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9555_stimuli.txt",
    9556  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9556_stimuli.txt",
    9799  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9799_stimuli.txt",
    9816  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9816_stimuli.txt",
    9833  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9833_stimuli.txt",
    9850  : "/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/Mod_stimuli/Mod9850_stimuli.txt",

}

mod_stimuli = {}
for modhash in mod_stimuli_files:
    mod_stimuli[modhash] = np.loadtxt(
        mod_stimuli_files[modhash],
        dtype = 'str',
        usecols=range(0,48), # always 48 tcs per module
    )

with open('/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TClist.txt', 'a') as file_out:
    for evt in range(nevts):
        file_out.write('Event{}\n'.format(evt))
        print(evt)
        for modhash in mod_stimuli:
            file_out.write('Module{} : '.format(modhash))
            for tc_e in mod_stimuli[modhash][evt,:]:
                file_out.write('{} '.format(tc_e))
            file_out.write('\n')
        
mod_stimuli_per_evt = {'event' : []}
for evt in range(nevts):
    mod_stimuli_per_evt['event'].append({})
    mod_stimuli_per_evt['event'][evt]['module'] = {}
    for modhash in mod_stimuli:
        mod_stimuli_per_evt['event'][evt]['module'][modhash] = {}        
        mod_stimuli_per_evt['event'][evt]['module'][modhash]['tc'] = []
        for tc_e in mod_stimuli[modhash][evt,:]:
            mod_stimuli_per_evt['event'][evt]['module'][modhash]['tc'].append(tc_e)
        #[tc_e for tc_e in mod_stimuli[modhash][evt,:]]
        
        print(mod_stimuli_per_evt['event'][evt]['module'][modhash]['tc'])

import json
with open('/eos/user/l/lportale/HGCTPG/HGCTPGEmulatorTester/inputs/TClist.json','w') as file_out:
    json.dump(mod_stimuli_per_evt, file_out, indent=4)
   
   
   
   
        
        
