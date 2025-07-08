import sys, argparse

import numpy as np
from PIL import Image
import noise
import math

def main(argv):

    parser = argparse.ArgumentParser()
    parser.add_argument('-r', '--resolution',
                        type = int,
                        default = 1024)
    parser.add_argument('--octaves',
                        type = int,
                        default = 6)
    parser.add_argument('-o', '--output',
                        required = True)
    args = parser.parse_args()

    shape = (args.resolution, args.resolution)
    scale = 0.2

    x_idx = np.linspace(0, 1, shape[0])
    y_idx = np.linspace(0, 1, shape[1])
    world_x, world_y = np.meshgrid(x_idx, y_idx)

    data = np.vectorize(noise.pnoise2)(world_x / scale,
                                       world_y / scale,
                                       octaves = args.octaves,
                                       repeatx = 1.0 / scale,
                                       repeaty = 1.0 / scale)

    offset = np.min(data)
    scale = 255.0 / (np.max(data) - np.min(data))
    data = np.clip(np.floor(4 * scale * (data - 0.5 * offset)), a_min = 0, a_max = 255).astype(np.uint8)

    img = Image.fromarray(data)
    img.save(args.output)

if __name__ == "__main__":
    main(sys.argv[1:])
