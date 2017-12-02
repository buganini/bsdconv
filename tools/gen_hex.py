#!/usr/bin/env python3
import sys

if len(sys.argv) != 3:
    print("Usage: {} fromfile tofile".format(sys.argv[0]))
    sys.exit(1)

fromfile = sys.argv[1]
tofile = sys.argv[2]

m = {
"0": "0",
"1": "1",
"2": "2",
"3": "3",
"4": "4",
"5": "5",
"6": "6",
"7": "7",
"8": "8",
"9": "9",
"A": "aA",
"B": "bB",
"C": "cC",
"D": "dD",
"E": "eE",
"F": "fF",
}

with open(tofile, "w") as tof:
    for i in range(256):
        hh = "{:02X}".format(i)
        bb = "".join(["{:02X}".format(ord(c)) for c in hh])
        tof.write("03{}\t{}\n".format(hh, bb))

with open(fromfile, "w") as fromf:
    for i in range(256):
        hh = "{:02X}".format(i)
        hhs = [""]
        for c in hh:
            nhh = []
            for x in m[c]:
                for kk in hhs:
                    nhh.append(kk+x)
            hhs = nhh

        for hh in hhs:
            bb = "".join(["{:02X}".format(ord(c)) for c in hh])
            fromf.write("{}\t03{}\n".format(bb, hh))
