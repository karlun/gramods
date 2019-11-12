#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script for Gimp - run with:
# gimp -idf --batch-interpreter python-fu-eval -b "import sys; sys.path=['.']+sys.path; import fur_texture; fur_texture.run()" -b "pdb.gimp_quit(1)"

import os, glob, sys, time
from gimpfu import *

def run(output):

  gimp.set_background((0, 0, 0))

  size = 1024
  image = gimp.Image(size, size, RGB)

  lG = gimp.Layer(image, "G", size, size, RGB_IMAGE, 100, NORMAL_MODE)
  image.insert_layer(lG)
  pdb.plug_in_solid_noise(image, lG, False, False, 846513215, 15, 16, 16)

  pdb.script_fu_predator(image, lG, 0, False, 1, False, False)

  pdb.gimp_equalize(lG, False)

  pdb.gimp_file_save(image, lG, output, '?')
  pdb.gimp_image_delete(image)

if __name__ == "__main__":
  print "Running as __main__ with args: %s" % sys.argv
