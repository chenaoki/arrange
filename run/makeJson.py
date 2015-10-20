#!/usr/bin/env python

import  commands, sys, os
import mysql.connector
from datetime import date, datetime, timedelta

if __name__ == '__main__':

    if not len( sys.argv ) is 5:
        print 'usage : makeJson [source directory]  [save directory]  [analysis engine] [query where]'
        print 'ex:'
        print 'makeJson \n\t/Volumes/Recordings/ExperimentData \n\t/Volumes/Recordings/AnalysisResult \n\toptical \n\t"WHERE sample_id = 1"'
        exit()

    sourceDirPath = sys.argv[1]
    saveDirPath = sys.argv[2]
    engine =  sys.argv[3]
    where =  sys.argv[4]

    # Connection to MySQL Server
    con = mysql.connector.connect(
        host='192.168.36.15',
        db='BMPE_ARR',
        user='tomii',
        passwd='feedback',
        buffered=True)
    cur = con.cursor(dictionary=True)

    cur.execute( 'select * from sessions %s'  % where)
    session_list = cur.fetchall()

    for ses in session_list:
        cur.execute( 'select * from samples where id = %s' % ses["sample_id"])
        sample_info = cur.fetchone()
        cur.execute( 'select * from cameras where id = %s' % sample_info["camera_id"])
        camera_info = cur.fetchone()

        #print ses
        #print sample_info
        #print camera_info

        # file name prefix setting
        if camera_info["name"] == "max":
            filePrefix =ses["name"].lower()
        if camera_info["name"] == "sa4":
            #filePrefix = ses["name"]
            filePrefix = 'raw/cam/'
        if camera_info["name"] == "dalsa":
            filePrefix = "f_"
        filePrefix = filePrefix.replace('@', '')

        # edit template
        f =  open("template/%s_%s.template" % ( engine, camera_info["name"] ), "r" )
        fileStr = f.read()
        f.close()

        fileStr = fileStr.replace('SOURCE_DIR', sourceDirPath)
        fileStr = fileStr.replace('SAVE_DIR', saveDirPath)
        fileStr = fileStr.replace('SAMPLE_NAME', sample_info["name"])
        fileStr = fileStr.replace('SESSION_NAME', ses["name"])
        fileStr = fileStr.replace('FILE_PREFIX', filePrefix)
        fileStr = fileStr.replace('IMG_SIZE', str(ses["img_size"]))
        #if sample_info["mask"] > 0:
        fileStr = fileStr.replace('MASK_IMG_NAME', sample_info["name"])
        #else:
            #fileStr = fileStr.replace('MASK_IMG_NAME', "white")

        with open('%s_%s_%s_%s.json' % (engine, camera_info["name"],  sample_info["name"], ses["name"] ) , 'w') as outf:
            outf.write(fileStr)
