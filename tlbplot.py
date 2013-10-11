#!/usr/bin/env python

'''
Make a cdf for 
'''

import sys
import re
import math
import numpy as p
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from matplotlib import rc

def main(infile, outname):
    ticks_font = matplotlib.font_manager.FontProperties(family='sans-serif', style='normal', size=12, weight='normal', stretch='normal')

    fig = plt.figure()
    ax = fig.add_subplot('111')
    ax.ticklabel_format(style='plain')

    xvals = []
    yvals = []
    with open(infile) as thefile:
        for line in thefile:
            if re.match('^\s*#', line):
                continue
            pages,cycles = [ int(x) for x in line.split() ]
            xvals.append(pages)
            yvals.append(cycles)

    ax.semilogx(xvals, yvals, basex=2, color='b', marker='.')
    ax.set_xlabel('number of pages')
    ax.set_ylabel('latency (cycles)')
    ax.set_title('TLB size measurement')

    ax.set_xlim(1, max(xvals)+1)
    ax.set_ylim(0, max(yvals)*1.1)

    ax.grid(True)
    plt.savefig(outname + '.pdf', bbox_inches="tight", pad_inches=0.1)
    plt.close()    

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "Usage: {} <infile> <outname>".format(sys.argv[0])
        sys.exit()
    main(sys.argv[1], sys.argv[2])
