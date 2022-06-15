# !/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Apr  1 10:49:31 2021

@author: adinale

Convert sequences collected from a Samsung DVS Gen3 to a YARP-friendly format.

The sequences are stored as *.txt and have the following format:
    - 1st row = sensors dimensions as [dimX dimY]
    - other rows = dvs events expressed as [ts x y pol]

"""

# %% Preliminaries
import os
import sys
import pandas as pd
import numpy as np
from bimvee.exportIitYarp import exportIitYarp

# Get local environment variables
bimvee_path = os.environ.get('BIMVEE_PATH')
datasets_path = os.environ.get('DATASETS_PATH')

# Add local paths
sys.path.insert(0, bimvee_path)

# %% Import Gen3 sequences

dataset_name = str(sys.argv[1])
filePathOrName = os.path.join(datasets_path, dataset_name + '.txt')

sensor_dimensions = pd.read_csv(filePathOrName, sep=" ", nrows=1,
                                header=None, names=('dimX', 'dimY'),
                                dtype={'dimX': np.uint16, 'dimY': np.uint16})

imported_df = pd.read_csv(filePathOrName,
                          skiprows=1,
                          sep=" ",
                          header=None,
                          names=('ts', 'x', 'y', 'pol'),
                          dtype={'ts': np.float64,
                                 'x': np.uint16,
                                 'y': np.uint16,
                                 'pol': np.int16})

# %% Create a bimvee container and convert polarities to bool for bimvee

polarity = np.zeros(len(imported_df['pol']), dtype=bool)

for idx, p in enumerate(imported_df['pol']):
    if p == 1:
        polarity[idx] = True  # events with -1 polarity become False

imported = {'data': {'gen3': {'dvs':
                              {'ts': (imported_df['ts'] - imported_df['ts'][0]).to_numpy(),
                               'x': imported_df['x'].to_numpy(),
                               'y': imported_df['y'].to_numpy(),
                               'pol': polarity,
                               'dimX': sensor_dimensions['dimX'][0],
                               'dimY': sensor_dimensions['dimY'][0]}}},
            'info':
                {'filePathOrName': filePathOrName}}

# %% Export imported dataset back to YARP bottles format

exportIitYarp(imported,
              exportFilePath=os.path.join(datasets_path,
                                          dataset_name + "_converted"),
              pathForPlayback=os.path.join(datasets_path,
                                           dataset_name + "_converted"),
              dataTypes=['dvs'],
              protectedWrite=False)
