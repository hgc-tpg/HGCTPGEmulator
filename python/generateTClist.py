import numpy as np

nevts = 28

mod_stimuli_files = {
    10292 : "../inputs/Mod_stimuli/Mod10292_stimuli.txt",
    10306 : "../inputs/Mod_stimuli/Mod10306_stimuli.txt",
    11056 : "../inputs/Mod_stimuli/Mod11056_stimuli.txt",
    11570 : "../inputs/Mod_stimuli/Mod11570_stimuli.txt",
    12355 : "../inputs/Mod_stimuli/Mod12355_stimuli.txt",
    3361  : "../inputs/Mod_stimuli/Mod3361_stimuli.txt",
    3378  : "../inputs/Mod_stimuli/Mod3378_stimuli.txt",
    3858  : "../inputs/Mod_stimuli/Mod3858_stimuli.txt",
    3859  : "../inputs/Mod_stimuli/Mod3859_stimuli.txt",
    5411  : "../inputs/Mod_stimuli/Mod5411_stimuli.txt",
    6930  : "../inputs/Mod_stimuli/Mod6930_stimuli.txt",
    6931  : "../inputs/Mod_stimuli/Mod6931_stimuli.txt",
    7456  : "../inputs/Mod_stimuli/Mod7456_stimuli.txt",
    7473  : "../inputs/Mod_stimuli/Mod7473_stimuli.txt",
    8497  : "../inputs/Mod_stimuli/Mod8497_stimuli.txt",
    9521  : "../inputs/Mod_stimuli/Mod9521_stimuli.txt",
    9522  : "../inputs/Mod_stimuli/Mod9522_stimuli.txt",
    9795  : "../inputs/Mod_stimuli/Mod9795_stimuli.txt",
}

mod_stimuli = {}
for modhash in mod_stimuli_files:
    mod_stimuli[modhash] = np.loadtxt(
        mod_stimuli_files[modhash],
        dtype = 'str',
        usecols=range(0,48), # always 48 tcs per module
    )

with open('./TClist.txt', 'a') as file_out:
    for evt in range(nevts):
        file_out.write('Event{}\n'.format(evt))
        for modhash in mod_stimuli:
            file_out.write('Module{} : '.format(modhash))
            for tc_e in mod_stimuli[modhash][evt,:]:
                file_out.write('{} '.format(tc_e))
            file_out.write('\n')

