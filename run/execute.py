#!/usr/bin/env python

import commands
import os
import sys

if __name__ ==  '__main__':

    if len(sys.argv) != 2:
        print 'Syntax'
        print './execute.py [engine name]'
        exit()
    engine = sys.argv[1]
    files = commands.getoutput('ls ./%s*.json' % engine).replace('\n', ' ').split(' ')
    for f in files:
        print f
    for i, f in enumerate(files):
        cmd = "Arrange -e %s -p %s" %( engine, f )
        print "%d/%d : " % ( i+1, len(files) ), cmd
        os.system(cmd)
        print "done"
