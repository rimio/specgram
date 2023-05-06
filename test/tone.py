#
# Copyright (c) 2020-2023 Vasile Vilvoiu <vasi@vilvoiu.ro>
#
# specgram is free software; you can redistribute it and/or modify
# it under the terms of the MIT license. See LICENSE for details.
#

import argparse
import numpy as np
import time
import sys

def main():
    # Argument parsing
    parser = argparse.ArgumentParser(description='Generate a tone on stdout.')
    parser.add_argument('freq', type=float, help='frequency of tone')
    parser.add_argument('rate', type=float, help='sampling rate')
    parser.add_argument('dtype', type=str, help='data type')
    parser.add_argument('--bs', type=int, default=512, help='block size')
    parser.add_argument('--pp', type=float, default=2.0, help='peak to peak amplitude')
    #parser.add_argument('--rl', action='store_true', help='rate limit output')
    args = parser.parse_args()

    # Parse datatype
    if args.dtype[0] == 'c':
        is_complex = True
        args.dtype = args.dtype[1:]
    else:
        is_complex = False

    if args.dtype == 'u8':
        dtype = np.uint8
    elif args.dtype == 's8':
        dtype = np.int8
    elif args.dtype == 'u16':
        dtype = np.uint16
    elif args.dtype == 's16':
        dtype = np.int16
    elif args.dtype == 'u32':
        dtype = np.uint32
    elif args.dtype == 's32':
        dtype = np.int32
    elif args.dtype == 'u64':
        dtype = np.uint64
    elif args.dtype == 's64':
        dtype = np.int64
    elif args.dtype == 'f32':
        dtype = np.float32
    elif args.dtype == 'f64':
        dtype = np.float64
    else:
        raise Exception('bad datatype')

    index = 0
    while True:
        # Generate complex tone
        t = np.linspace(index, index + args.bs - 1, args.bs, dtype=np.float128) / args.rate
        c = np.exp(2j * np.pi * args.freq * t) / 2.0 # -0.5 .. 0.5

        # Convert to output data type
        if args.dtype[0] == 's':
            dr = (np.real(c) * args.pp * np.iinfo(dtype).max).astype(dtype)
            di = (np.imag(c) * args.pp * np.iinfo(dtype).max).astype(dtype)
        elif args.dtype[0] == 'u':
            dr = (np.real((c + 0.5) / 2.0) * args.pp * np.iinfo(dtype).max).astype(dtype)
            di = (np.imag((c + 0.5) / 2.0) * args.pp * np.iinfo(dtype).max).astype(dtype)
        else:
            dr = np.real(c * args.pp).astype(dtype)
            di = np.imag(c * args.pp).astype(dtype)

        # Output block
        if (is_complex):
            dc = np.ravel(np.column_stack((dr, di)))
            sys.stdout.buffer.write(dc.tobytes())
        else:
            sys.stdout.buffer.write(dr.tobytes())
        sys.stdout.buffer.flush()

        # Advance
        index += args.bs

if __name__ == '__main__':
    main()
