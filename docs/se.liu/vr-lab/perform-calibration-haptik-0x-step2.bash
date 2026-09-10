#!/usr/bin/bash

echo
echo "Second step: calibration of screen position"
./bin/gm-tracker-registration \
    --output-template se.liu.haptics.head-tracked-mono-view-template-step2.x3d \
    --output-template se.liu.haptics.head-tracked-stereo-view-template-step2.x3d \
    --output-file se.liu.haptics.head-tracked-mono-view.x3d \
    --output-file se.liu.haptics.head-tracked-stereo-view.x3d \
    --config urn:gramods:config/output-normal-to-console.xml \
    --config urn:gramods:config/output-debug-to-file.xml \
    --config urn:gramods:config/se.liu/vr-lab.calibration-tracker-openvr.xml \
    --a-point -0.266,0.278,0.128 \
    --a-point -0.266,0.278,0.428 \
    --a-point +0.266,0.278,0.428 \
    --a-point +0.266,0.278,0.128 \
    --position-inlier-threshold 0.01 \
    --orientation-inlier-threshold 0.1
