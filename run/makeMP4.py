#!/usr/bin/env python

import os
import commands
import sys

targets = ['']

if __name__ == '__main__':

    if len(sys.argv) != 3:
        print 'Usage : makeMP4.py [ target directory ] [ frames per second ]'
        exit()

    d = sys.argv[1]
    files = commands.getoutput('find %s -type f -name *.jpg' % d).split('\n')

    #rename
    for n, f in enumerate(files):
        cmd = 'mv %s %s/rn_%05d.jpg' % ( f,  d, n )
        print cmd
        os.system(cmd)

    cmd = 'ffmpeg -r %s -i %s/rn_%%05d.jpg %s.mp4' % (  sys.argv[2], d, d)
    print cmd
    os.system(cmd)
