
import sys, argparse

import noise
import numpy as np
import scipy.signal
from PIL import Image, ImageFilter

def main(argv):

  parser = argparse.ArgumentParser()
  parser.add_argument('-r', '--resolution',
                      type = int,
                      default = 1024)
  parser.add_argument('-o', '--output',
                      required = True)
  args = parser.parse_args()

  shape = (args.resolution, args.resolution)
  scale = 0.05

  x_idx = np.linspace(0, 1, shape[0])
  y_idx = np.linspace(0, 1, shape[1])
  world_x, world_y = np.meshgrid(x_idx, y_idx)

  data = np.vectorize(noise.pnoise2)(world_x / scale,
                                     world_y / scale,
                                     octaves = 5,
                                     repeatx = 1.0 / scale,
                                     repeaty = 1.0 / scale)

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
  img = Image.fromarray(data)
  img = img.crop((1, 1, 1 + args.resolution, 1 + args.resolution))

  img.save(args.output)


if __name__ == "__main__":
  main(sys.argv[1:])
