"""
Created on Thu Mar 18 10:55:39 2021

@author: Leandro de Souza Rosa <leandro.desouzarosa@iit.it>
"""

"""
Copyright (C) 2019 Event-driven Perception for Robotics
This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with 
this program. If not, see <https://www.gnu.org/licenses/>.
"""

#%% Preliminaries - set your paths as necessary

import os, sys # A system-specific prefix, for working between linux and windows
prefix = 'C:/' if os.name == 'nt' else '/home/leandro/'
sys.path.append(os.path.join(prefix, 'gitRepos/bimvee')) # A path to this library
name = 'parking1'

#%%
from bimvee.importRpgDvsRos import importRpgDvsRos
from bimvee.split import cropTime
filePathOrName = os.path.join(prefix, 'data/ETH_HDR/'+name+'/'+name+'.bag')
#inspected = importRpgDvsRos(filePathOrName=filePathOrName)

template = {
    'ch0': {
        'dvs': '/dvs/cam1/events'
        }
    }

#imported = importRpgDvsRos(filePathOrName=filePathOrName, template=template)

    
#%%

imported = cropTime(importRpgDvsRos(filePathOrName=filePathOrName, 
                           template=template),
                           startTime = 16,
                           stopTime = 24)


#imported['data'] = imported['data'].pop('ch0')
#%% Choose to export only specific datatypes; 
# overwrite data if the export is already there

from bimvee.exportIitYarp import exportIitYarp

exportIitYarp(imported,
            exportFilePath = prefix+'data/ETH_HDR/'+name+'/'+name,
            pathForPlayback = prefix+'data/ETH_HDR/'+name+'/'+name,
            dataTypes = ['dvs'],
            protectedWrite = False)
