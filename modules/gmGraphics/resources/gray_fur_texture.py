#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Script for Gimp - run with:
# gimp -idf --batch-interpreter python-fu-eval -b "import sys; sys.path=['.']+sys.path; import gray_fur_texture; gray_fur_texture.run(size = 4096, output = 'gray_fur_texture.png')" -b "pdb.gimp_quit(1)"

import os, glob, sys, time
from gimpfu import *

def run(size, output):

  gimp.set_background((0, 0, 0))

  image = gimp.Image(size, size, RGB)

  lG = gimp.Layer(image, "G", size, size, RGB_IMAGE, 100, NORMAL_MODE)
  image.insert_layer(lG)
  pdb.plug_in_solid_noise(image, lG, False, False, 651321575, 15, 16, 16)
  pdb.plug_in_edge(image, lG, 1, 1, 0)

  pdb.gimp_equalize(lG, False)
  pdb.gimp_file_save(image, lG, output, '?')
  pdb.gimp_image_delete(image)

if __name__ == "__main__":
  print "Running as __main__ with args: %s" % sys.argv
