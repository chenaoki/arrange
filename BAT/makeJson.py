#!/usr/bin/env python

import os

#ids = [14,34,38]
ids = [6]
#ids = range(4,36)

for i in ids:
    os.system("sed -e \'s/XXXX/" + str(i).zfill(4) + "/g\' ./ArrangeParam.json > " + str(i).zfill(4) + ".json")

