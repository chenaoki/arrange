#!/usr/bin/env python

import os

endf = 1800
dirNum = [5, 6, 7, 9, 11, 13, 14, 15, 17, 20, 21, 25, 26, 33, 34]
#path = 'D:\\local\\tmp\\ExperimentData\\20111214-3\\C001S%04d -f %%s\\c001s%04d%%06d.raww'
path_p = './DerivedData/Arrange/Build/Products/Debug/'
program = path_p + 'Arrange'
path_s = '/usr/local/tmp/ExperimentData/20111214-3/'
path_d = '/usr/local/tmp/AnalysisResult/20111214-3/'
elecConf = path_s + 'elec.cnf'

if __name__ == '__main__':
	for i in dirNum:
		cmd = "%s -m full -c max -s %sC001S%04d -f %%s/c001s%04d%%06d.raww -d %sC001S%04d -b 1 -e %d -i 1 -p %s" % (program, path_s,i,i,path_d,i,endf, config)
		print cmd
		os.system( cmd )
