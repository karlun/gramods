#!/bin/bash

# Create new_config folder structure
rm -fr new_config
mkdir -p new_config/AppData_Local_openvr
mkdir -p new_config/Steam_config_lighthouse
mkdir -p new_config/Steam_config_vrappconfig

# Populate new_config folder structure with streamvr data
cp -r $HOME/AppData/Local/openvr new_config/AppData_Local_openvr
cp -r "/c/Program Files (x86)/Steam/config/lighthouse" new_config/Steam_config_lighthouse
cp -r "/c/Program Files (x86)/Steam/config/vrappconfig" new_config/Steam_config_vrappconfig

# Rotate folders
rm -rf steamvr_config.old
mv -f steamvr_config steamvr_config.old
mv -f new_config steamvr_config

# Copy to 
rm -rf /c/Apps/steamvr_config
cp -r steamvr_config /c/Apps/steamvr_config
