#!/usr/bin/env python

import commands
import os
import sys

if __name__ ==  '__main__':

    if len(sys.argv) != 2:
        print 'Syntax'
        print './execute.py [engine name]'
        exit()
    files = commands.getoutput('ls ./*.json').replace('\n', ' ').split(' ')
    print files
    for f in files:
        if f == "./ArrangeParam.json":
            continue
        print f
        os.system("Arrange -e %s -p %s" %( sys.argv[1], f ))
