#!/usr/bin/env python

import os

for i in range(5,40):
    os.system("sed -e \'s/XXXX/" + str(i+1).zfill(4) + "/g\' ./ArrangeParam.json > " + str(i+1).zfill(4) + ".json")

