import sys, argparse

import numpy as np
from PIL import Image
from noise import pnoise2
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

    image = Image.new('L', shape)
    pixels = image.load()

    for y in range(args.resolution):
        for x in range(args.resolution):
            nx = x / args.resolution
            ny = y / args.resolution
            
            noise_val = 0
            freq = 10.0
            amp = 1.0
            for _ in range(args.octaves):
                noise_val += amp * pnoise2(freq * nx, freq * ny)
                freq *= 2
                amp *= 0.5

            value = max(0, min(1, 1.5 * noise_val))
            gray = 255 - int(255 * pow(value, 2))
            pixels[x, y] = gray

    image.save(args.output)

if __name__ == "__main__":
    main(sys.argv[1:])
