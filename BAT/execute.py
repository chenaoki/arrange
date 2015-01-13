#!/usr/bin/env python

import commands
import os

files = commands.getoutput('ls ./*.json').replace('\n', ' ').split(' ')
print files
for f in files:
    if f == "./ArrangeParam.json":
        continue
    print f
    os.system("Arrange -e optical -p " + f)
