#!/usr/bin/env python

import os
import commands
import sys

targets = ['']

if __name__ == '__main__':

    if len(sys.argv) != 2:
        print 'Usage : makeMP4.py [ target directory ]'
        exit()

    jpgs = commands.getoutput('find %s -type d -name jpg' % sys.argv[1]).split('\n')

    for jpg in jpgs:
        dirs = commands.getoutput('find %s -type d -maxdepth 1 -mindepth 1' % jpg).split('\n')
        for d in dirs:
            files = commands.getoutput('find %s -type f -name *.jpg' % d).split('\n')
            if len(files) > 10:
                file_name = files[0].split('/')[-1]
                file_num_str = file_name.rstrip('.jpg').split('_')[1]
                file_num = int( file_num_str )
                if file_num != 1:
                    for i in range( 1, file_num) :
                        cmd =  'cp %s/%s %s/%s' % ( d, file_name,  d, file_name.replace( file_num_str, '%05d' % i ))
                        print cmd
                        os.system(cmd)

                file_fmt = file_name.replace(file_num_str, "%05d")
                cmd = 'ffmpeg -r 30 -i %s/%s %s.mp4' % (d, file_fmt, d.replace('/','_'))
                print cmd
                os.system(cmd)





