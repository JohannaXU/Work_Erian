#!/usr/bin/python

import mysql.connector
from mysql.connector import errorcode
from mysql.connector import MySQLConnection, Error

import time

from Header_DeviceType import read_macrecord, traverse_devicetype, insert_DT, update_status, time_diff, clear_tables, clear_deviceType, clear_deviceStatus
from header_trackdevice import read_info, format_csv

try:
	#hostname = '192.168.0.102'
	hostname = '192.168.0.116'
	username = 'root'
	password = 'root'
	database = 'HAS'

	db = mysql.connector.connect(user= username, password=password, host=hostname, database=database)

	if db.is_connected():
			print('connection established.')
	else:
			print('connection failed.')

	cursor = db.cursor()

	clear_para = 0

	# -- initialize the connection
	status_info = read_info(db, cursor)
	format_csv(db, cursor, status_info)

	while True:
		print('\n Start New LOOP')

		clear_para = clear_para + 1

		""" execute READ MACRecord """	
		raw_data = read_macrecord(db, cursor)

		""" check whether raw_data MAC is in the DeviceTable """
		traverse_devicetype(db, cursor, raw_data)

		# ----- debug for device_tracking ------ 
		#status_info = read_info(db, cursor)
		#format_csv(db, cursor, status_info)

		if clear_para == 30:
			clear_tables(db, cursor)
			clear_para = 0

			# ----- save status_info in file every 30 minutes ---
			status_info = read_info(db, cursor)
			format_csv(db, cursor, status_info)

		time.sleep(60)

except Error as error:
	print(error)

finally:
	cursor.close()
	db.close() 
	print('Mysql Database Connection closed.')