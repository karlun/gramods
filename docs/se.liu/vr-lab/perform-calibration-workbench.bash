#!/usr/bin/bash

./bin/gm-tracker-registration \
    --output-template se.liu.workbench.3DTV-Sony50-template-step2.xml \
    --output-file se.liu.workbench.3DTV-Sony50.xml \
		--config urn:gramods:config/output-normal-to-console.xml \
		--config urn:gramods:config/output-debug-to-file.xml  \
		--config urn:gramods:config/se.liu/workbench.calibration-tracker-openvr.xml   \
    --a-point -0.555,-0.312,0 \
    --a-point -0.555,+0.312,0 \
    --a-point +0.555,+0.312,0 \
    --a-point +0.555,-0.312,0 \
		--position-inlier-threshold 0.01   \
		--orientation-inlier-threshold 0.1
