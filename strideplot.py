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

def makelabel(strsize):
    bytes = int(strsize)
    i = 0
    factor = ['B','KB','MB','GB']
    while bytes / 1024 > 0:
        bytes /= 1024
        i += 1
    return '{}{}'.format(int(bytes),factor[i])

def read_next(infile):
    arr_size = None
    infile.readline()
    xvals = []
    yvals = []
    while True:
        line = infile.readline()
        if not line:
            break

        if re.match('^# running stride', line):
            break

        mobj = re.match('^# array size (\d+)', line)
        if mobj:
            arr_size = int(mobj.groups()[0])
            continue

        if re.match('^\s*#', line):
            continue

        vals = line.split()
        if len(vals) != 2:
            print "bad stuff:", line,vals
            assert(len(vals) == 2)

        xvals.append(int(vals[0]))
        yvals.append(int(vals[1]))

    return arr_size, xvals, yvals
        
# running stride with 134217728
# page size is 4096
# array size 268435456 (bytes)
# stride_length    cycles
# -------------    ------


def main(infile, outname):
    ticks_font = matplotlib.font_manager.FontProperties(family='sans-serif', style='normal', size=12, weight='normal', stretch='normal')

    fig = plt.figure()
    ax = fig.add_subplot('111')
    ax.ticklabel_format(style='plain')

    thefile = open(infile)
    ymax = 0
    ymin = 10e10
    xmax = 0
    xmin = 10e10

    colors = ['b','g','r','c','m','y','purple','orange','0.5','0.8']
    colors *= 2

    idx = 0
    text_locations = {}

    while True:
        size,xvals,yvals = read_next(thefile)
        if size is None:
            break
        fgcolor = colors.pop()
        ax.semilogx(xvals, yvals, basex=2, color=fgcolor, marker='.', label="{}".format(size))
        textx,texty = xvals[len(xvals)/2],yvals[len(yvals)/2]

        textlabel = makelabel(size)
        ax.text(textx, texty, textlabel, size=6, ha='right', va='center', rotation=45, bbox={'boxstyle':'round,pad=0.1', 'fc':fgcolor, 'ec':fgcolor})
        xmin = min(min(xvals), xmin)
        xmax = max(max(xvals), ymax)
        ymin = min(min(yvals), ymin)
        ymax = max(max(yvals), ymax)
        idx += 1

    ax.set_xlabel('stride')
    ax.set_ylabel('latency (cycles)') 
    ax.set_title('Effect of stride on memory hierarchy')

    ax.set_xlim(xmin, xmax)
    ybuffer = (ymax-ymin)*0.1
    ax.set_ylim(ymin-ybuffer,ymax+ybuffer)

    ax.grid(True)
    plt.savefig(outname + '.pdf', bbox_inches="tight", pad_inches=0.1)
    plt.close()    

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "Usage: {} <infile> <outname>".format(sys.argv[0])
        sys.exit()
    main(sys.argv[1], sys.argv[2])
