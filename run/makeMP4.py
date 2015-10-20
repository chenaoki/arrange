#!/usr/bin/env python

import os
import commands
import sys

targets = ['']

if __name__ == '__main__':

    if len(sys.argv) != 4:
        print 'Usage : makeMP4.py [ target directory ] [file name prefix ] [ frames per second ]'
        exit()

    d = sys.argv[1]
    files = commands.getoutput('find %s -type f -name *.jpg' % d).split('\n')

    #rename
    if False:
	for n, f in enumerate(files):
		cmd = 'mv %s %s/rn_%05d.jpg' % ( f,  d, n )
		print cmd
		os.system(cmd)

    cmd = 'ffmpeg -y -r %s -i %s/%s%%05d.jpg %s.mp4' % (  sys.argv[3], d, sys.argv[2], d)
    print cmd
    os.system(cmd)
