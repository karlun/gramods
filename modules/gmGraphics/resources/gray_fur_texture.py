
import sys, getopt

import noise
import numpy as np
import scipy.signal
from PIL import Image, ImageFilter

def main(argv):

  try:
    opts, args = getopt.getopt(argv,"hr:o:",["resolution=","output="])
  except getopt.GetoptError:
    print('gray_fur_texture.py -r <resolution> -o <outputfile>')
    sys.exit(2)

  resolution = 1024
  outputfile = None

  for opt, arg in opts:
    if opt == '-h':
      print('gray_fur_texture.py -r <resolution> -o <outputfile>')
      sys.exit()
    elif opt in ("-r", "--resolution"):
      resolution = int(arg)
    elif opt in ("-o", "--output"):
      outputfile = arg

  if outputfile is None:
    print('gray_fur_texture.py -r <resolution> -o <outputfile>')
    sys.exit(2)

  shape = (resolution, resolution)
  scale = 0.05

  x_idx = np.linspace(0, 1, shape[0])
  y_idx = np.linspace(0, 1, shape[1])
  world_x, world_y = np.meshgrid(x_idx, y_idx)

  data = np.vectorize(noise.pnoise2)(world_x / scale,
                                     world_y / scale,
                                     octaves = 5,
                                     repeatx = 1/scale,
                                     repeaty = 1/scale)

  offset = np.min(data)
  scale = 255.0 / (np.max(data) - np.min(data))
  data = (data - offset) * scale

  data_x = scipy.signal.convolve2d(data,
                                   [[ 1, 0, -1 ],
                                    [ 2, 0, -2 ],
                                    [ 1, 0, -1 ]],
                                   mode = 'full',
                                   boundary = 'wrap')
  data_y = scipy.signal.convolve2d(data,
                                   [[ 1, 2, 1 ],
                                    [ 0, 0, 0 ],
                                    [ -1, -2, -1 ]],
                                   mode = 'full',
                                   boundary = 'wrap')
  data = np.sqrt(data_x * data_x + data_y * data_y)

  offset = np.min(data)
  scale = 255.0 / (np.max(data) - np.min(data))

  data = np.floor(scale * (data - offset)).astype(np.uint8)
  img = Image.fromarray(data, mode='L')

  img.save(outputfile)


if __name__ == "__main__":
  main(sys.argv[1:])
