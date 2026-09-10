#!/usr/bin/bash

echo
echo "First step: calibration of eye offset"
./bin/gm-tracker-registration \
    --output-template urn:gramods:config/se.liu/haptics.head-tracked-mono-view-template.x3d \
    --output-file se.liu.haptics.head-tracked-mono-view-template-step2.x3d \
    --output-template urn:gramods:config/se.liu/haptics.head-tracked-stereo-view-template.x3d \
    --output-file se.liu.haptics.head-tracked-stereo-view-template-step2.x3d \
    --config urn:gramods:config/output-normal-to-console.xml \
    --config urn:gramods:config/output-debug-to-file.xml \
    --config urn:gramods:config/se.liu/vr-lab.calibration-tracker-openvr-relative.xml \
    --a-point -0.05,0,0 \
    --a-point +0.05,0,0 \
    --a-point 0,-0.12,0 \
    --position-inlier-threshold 0.005 \
    --orientation-inlier-threshold 0.05
