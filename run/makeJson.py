#!/usr/bin/env python

import  commands, sys, os
import mysql.connector
from datetime import date, datetime, timedelta

if __name__ == '__main__':

    if not len( sys.argv ) is 5:
        print 'usage : makeJson [source directory]  [save directory]  [analysis engine]  [camera name]'
        print 'ex:'
        print 'makeJson /Volumes/Recordings/ExperimentData ./ optical sa4'
        exit()

    sourceDirPath = sys.argv[1] # ex. "/Volumes/Recordings/AnalysisResult"
    saveDirPath = sys.argv[2]
    engine =  sys.argv[3]
    camera_name = sys.argv[4]

    # Connection to MySQL Server
    con = mysql.connector.connect(
        host='192.168.36.15',
        db='BMPE_ARR',
        user='tomii',
        passwd='feedback',
        buffered=True)
    cur = con.cursor()

    cur.execute("select id from cameras where name = \"%s\"" % camera_name )
    camera_id = cur.fetchone()[0]
    print
    cur.execute( 'select id from samples where camera_id = \"%s\"'  % camera_id)
    sample_list = [ res[0] for res in cur.fetchall() ]

    print camera_id
    print sample_list

    for sample_id in sample_list:

        cur.execute( 'select name, mask from samples where id = \"%s\"' % sample_id)
        res = cur.fetchone()
        sample_name = res[0]
        flg_mask = int(res[1])
        cur.execute( 'select id, name, img_size from sessions where sample_id = \"%s\" and spiral = 1' % sample_id )
        session_list = [ {"id" : res[0], "name" : res[1], "img_size" : res[2] }  for res in cur.fetchall()] # (id, name) tupple

        print sample_name

        for ses in session_list:

            print ses["name"]

            if camera_name == "max":
                filePrefix =ses["name"].lower()
            if camera_name == "sa4":
                filePrefix = ses["name"]
            if camera_name == "dalsa":
                filePrefix = "f_"
            filePrefix = filePrefix.replace('@', '')

            img_size = ses["img_size"]

            cmd = "sed -e \'s/SOURCE_DIR/%s/g\' template/%s_%s.template >  temp1.json" % (  sourceDirPath.replace('/', '\/'), engine, camera_name  )
            print cmd
            os.system(cmd)
            cmd = "sed -e \'s/SAVE_DIR/%s/g\' temp1.json >  temp2.json" % saveDirPath.replace('/', '\/')
            print cmd
            os.system(cmd)
            cmd = "sed -e \'s/SAMPLE_NAME/%s/g\' temp2.json >  temp1.json" % sample_name
            print cmd
            os.system(cmd)
            cmd = "sed -e \'s/SESSION_NAME/%s/g\' temp1.json >  temp2.json" % ses["name"]
            print cmd
            os.system(cmd)
            cmd = "sed -e \'s/FILE_PREFIX/%s/g\' temp2.json >  temp1.json" % filePrefix
            print cmd
            os.system(cmd)
            cmd = "sed -e \'s/IMG_SIZE/%s/g\' temp1.json >  temp2.json" % img_size
            print cmd
            os.system(cmd)
            if flg_mask > 0:
                cmd = "sed -e \'s/MASK_IMG_NAME/%s/g\' temp2.json >  temp1.json" % sample_name
                print cmd
                os.system(cmd)
            else:
                cmd = "sed -e \'s/MASK_IMG_NAME/%s/g\' temp2.json >  temp1.json" % "white"
                print cmd
                os.system(cmd)
            cmd = "cp temp1.json %s_%s_%s_%s.json" % ( engine, camera_name,  sample_name, ses["name"] )
            print cmd
            os.system(cmd)
            os.system('rm -f temp*.json')

