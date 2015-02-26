#!/usr/bin/env python

import os
import commands
import sys

targets = ['']

if __name__ == '__main__':

    if len(sys.argv) != 3:
        print 'Usage : makeMP4.py [ target directory ] [frames per second]'
        exit()

    jpgs = commands.getoutput('find %s -type d -name jpg' % sys.argv[1]).split('\n')

    for jpg in jpgs:
        dirs = commands.getoutput('find %s -type d -maxdepth 1 -mindepth 1' % jpg).split('\n')
        for d in dirs:
            files = commands.getoutput('find %s -type f -name *.jpg' % d).split('\n')

            for n, f in enumerate(files):
                cmd = 'mv %s %s/rn_%05d.jpg' % ( f,  d, n )
                print cmd
                os.system(cmd)

            cmd = 'ffmpeg -r %s -i %s/rn_%%05d.jpg %s.mp4' % (  sys.argv[2], d, d)
            print cmd
            os.system(cmd)