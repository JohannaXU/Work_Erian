#!/usr/bin/python

import mysql.connector
from mysql.connector import errorcode
from mysql.connector import MySQLConnection, Error

import csv
import datetime
from datetime import timedelta

WHITE	=	3
GRAY	=	2
BLACK	=	1
LOCK	=	0

""" - Read information from DeviceStatus - """
def read_info(db, cursor):
	query_read_DeviceStatus = "SELECT MACAdd, CUR_Time, CUR_Position, CUR_Status, PRE_Status FROM DeviceStatus"

	info = []

	cursor.execute(query_read_DeviceStatus)
	row = cursor.fetchone()

	while row is not None:
		info.append(row)
		row = cursor.fetchone()

	length = len(info)
	for i in range(0, length):
		if info[i][3] != info[i][4]:
			item_csv(db, cursor, info[i])
		else:
			pass

	return info

def format_csv(db, cursor, info):
	#print(info)
	length = len(info)

	for i in range(0, length):
		macadd = info[i][0]
		#---------test
		#time = info[i][1]
		#print(time)
		#time = time + timedelta(hours=8)
		#print(time)

		position = info[i][2]
		status = info[i][3]

		time_now = datetime.datetime.now()
		#print(time_now)
		#-- test timedelta
		#time_now = time_now + timedelta(hours=8)

		if position == None:
			position = 0
		else:
			pass

		dict_info = {
			"TIME":	time_now.strftime('%Y-%m-%d %H:%M:%S'),
			"STATUS":	status,	
			"POSITION":	position,	
		}
		print(dict_info)

		filename = './data/' + macadd + '.csv'

		with open(filename, 'a') as f:
		#	writer = csv.DictWriter(f, dict_info.keys())
			writer = csv.writer(f)
		#	writer.writeheader()
			writer.writerow(dict_info.values())

def item_csv(db, cursor, item_info):
	macadd = item_info[0]

	time = item_info[1]
	time = time + timedelta(hours=8)

	position = item_info[2]
	status = item_info[3]

	if position == None:
		position = 0
	else:
		pass

	dict_info = {
			"TIME":	time.strftime('%Y-%m-%d %H:%M:%S'),
			"STATUS":	status,
			"POSITION":	position,		
		}
	print(dict_info)

	filename = './data/' + macadd + '.csv'

	with open(filename, 'a') as f:
		writer = csv.writer(f)
		writer.writerow(dict_info.values())

def new_item_csv(db, cursor, new_info):
	#print(new_info)

	macadd = new_info[0]

	time = new_info[1]
	time = time + timedelta(hours=8)

	position = new_info[2]

	status = WHITE

	if position == None:
		position = 0
	else:
		pass

	dict_info = {
			"TIME":	time.strftime('%Y-%m-%d %H:%M:%S'),
			"STATUS":	status,
			"POSITION":	position,		
		}
	print(dict_info)

	filename = './data/' + macadd + '.csv'

	with open(filename, 'a') as f:
		writer = csv.writer(f)
		writer.writerow(dict_info.values())









