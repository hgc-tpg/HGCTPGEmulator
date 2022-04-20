import numpy as np
import optparse
from os import listdir
from os.path import isfile, join
import json
 
def read_inputs(input_dir):
    stimuli = {}
    input_files = [f for f in listdir(input_dir) if isfile(join(input_dir,f))]
    for f in input_files:
        modhash = "".join((filter(str.isdigit, f)))
        stimuli[modhash] = np.loadtxt(
            join(input_dir,f),
            dtype = 'str',
            usecols = range(0,48) # always 48 TCs per module
        )
    return stimuli

def order_per_bx(stimuli, Nbx):
    stimuli_per_bx = {'event' : []}
    for bx in range(Nbx):
        stimuli_per_bx['event'].append({})
        stimuli_per_bx['event'][bx]['module'] = {}
        for modhash in stimuli:
            stimuli_per_bx['event'][bx]['module'][modhash] = {}
            stimuli_per_bx['event'][bx]['module'][modhash]['tc'] = []
            for tc_e in stimuli[modhash][bx,:]:
                stimuli_per_bx['event'][bx]['module'][modhash]['tc'].append(tc_e)
    return stimuli_per_bx         
   
if __name__=='__main__':
    parser = optparse.OptionParser()
    parser.add_option("--nbx", type="int",    dest="nbx",        help="Number of Bx in stimuli files")
    parser.add_option("--in",  type="string", dest="input_dir",  help="Input directory")
    parser.add_option("--out", type="string", dest="output_dir", help="Output directory")
    opt, args = parser.parse_args()

    mod_stimuli = read_inputs(opt.input_dir)
    mod_stimuli = order_per_bx(mod_stimuli, opt.nbx)
    
    output_file = opt.output_dir+'/TClist.json'
    with open(output_file,'w') as f_out:    
        json.dump(mod_stimuli, f_out, indent=4)

        
