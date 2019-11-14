#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script for Gimp - run with:
# gimp -idf --batch-interpreter python-fu-eval -b "import sys; sys.path=['.']+sys.path; import color_cubes_texture; color_cubes_texture.run(size = 4096, output = 'color_cubes_texture.png')" -b "pdb.gimp_quit(1)"

import os, glob, sys, time
from gimpfu import *

def run(size, output):

  gimp.set_background((0, 0, 0))

  image = gimp.Image(size, size, RGB)

  background = gimp.Layer(image, "background", size, size,
                          RGB_IMAGE, 100, NORMAL_MODE)
  background.fill(BACKGROUND_FILL)
  image.insert_layer(background)

  lR = gimp.Layer(image, "R", size, size, RGB_IMAGE, 100, ADDITION_MODE)
  image.insert_layer(lR)
  pdb.plug_in_solid_noise(image, lR, False, False, 846513215, 15, 16, 16)
  pdb.plug_in_colors_channel_mixer(image, lR, False,
                                   1.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0)

  lG = gimp.Layer(image, "G", size, size, RGB_IMAGE, 100, ADDITION_MODE)
  image.insert_layer(lG)
  pdb.plug_in_solid_noise(image, lG, False, False, 651321575, 15, 16, 16)
  pdb.plug_in_colors_channel_mixer(image, lG, False,
                                   0.0, 0.0, 0.0,
                                   0.0, 1.0, 0.0,
                                   0.0, 0.0, 0.0)

  lB = gimp.Layer(image, "B", size, size, RGB_IMAGE, 100, ADDITION_MODE)
  image.insert_layer(lB)
  pdb.plug_in_solid_noise(image, lB, False, False, 132157143, 15, 16, 16)
  pdb.plug_in_colors_channel_mixer(image, lB, False,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0,
                                   0.0, 0.0, 1.0)

  final_layer = image.flatten()
  pdb.plug_in_cubism(image, final_layer, size/128.0, 1.6, 0)

  pdb.gimp_file_save(image, final_layer, output, '?')
  pdb.gimp_image_delete(image)

if __name__ == "__main__":
  print "Running as __main__ with args: %s" % sys.argv
