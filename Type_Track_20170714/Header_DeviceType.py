#!/usr/bin/python

import mysql.connector
from mysql.connector import errorcode
from mysql.connector import MySQLConnection, Error

from header_trackdevice import new_item_csv

import time
import datetime

WHITE	=	3
GRAY	=	2
BLACK 	=	1
LOCK 	=	0

""" --------------- execute READ MACRecord -------------------- """
def read_macrecord(db, cursor):
	#query_read_MACRecord = ("SELECT MACAdd, Timestamp, Position from MACRecord where Record=1")
	query_read_MACRecord = ("SELECT MACAdd, Timestamp, Position from MACRecord")
	result = []

	#cursor = db.cursor()
	cursor.execute(query_read_MACRecord)

	row = cursor.fetchone()
	#print(row)

	while row is not None:
		#print(row)
		result.append(row)
		row = cursor.fetchone()

	return result

""" -------------- execute READ MAC from DeviceType ----------------- """
def traverse_devicetype(db, cursor, raw_data):
	query_DeviceType_MAC = ("SELECT MACAdd, Time FROM DeviceType")
	query_Previous_info = ("SELECT * FROM DeviceType WHERE MACAdd=%s")
	length = len(raw_data)
	print("The element number in MACRecord is: %d" % length)

	

	for i in range(0, length):
		MAC_raw = raw_data[i][0]
		time_raw = raw_data[i][1]
		
		#print(raw_data[i])

		num = 0

		cursor.execute(query_DeviceType_MAC)
		row = cursor.fetchone()

		""" traverse the DeviceType table """
		if row==None:
			print("row is NULL")
			insert_DT(db, cursor, raw_data[i])
			continue

		while row is not None:
			MAC_intable = row[0]
			time_intable =  row[1]
			
			row = cursor.fetchone()
			if MAC_raw == MAC_intable:
				num = 1

		if num == 1:
			#print('%dth MAC_raw\t %s \tis in the list' % (i,MAC_raw))
		## -- in order to check whether the raw_data is updated  -- ##
			if time_raw > time_intable:
				print("time from MACRecord:\t %s " % time_raw)
				print("time from DeviceType:\t %s \n" % time_intable)
				cursor.execute(query_Previous_info, (MAC_raw,))
				row = cursor.fetchone()
				update_status(db, cursor, raw_data[i], row)
			else:
				pass
		else:
			print("%dth MAC_raw\t %s \tis new.   Will be added to DeviceType Table" % (i,MAC_raw))
			insert_DT(db, cursor, raw_data[i])
			insert_DS(db, cursor, raw_data[i])
			# --- new device, save status_info into files at once ---
			new_item_csv(db, cursor, raw_data[i])
			print("\tNew Item STATUS_Info is saved as csv")


""" Add new MAC Info into DeviceType table """
def insert_DT(db, cursor, new_data):
	#print("Check!", new_data)
	query_insert_DeviceType = "INSERT INTO DeviceType(MACAdd, Time, StaticTime, Position, Del_R, Lock_R, STATUS, LIVE) VALUES(%s,%s,%s,%s,%s,%s,%s,%s)"

	macadd = str(new_data[0])
	time = new_data[1]
	time.strftime('%Y-%m-%d %H:%M:%s')
	statictime = str(0)
	delete_record = str(0)
	lock_record = str(0)
	status = str(WHITE)
	live = str(1)
	position = new_data[2]

	args = (macadd, time, statictime, position, delete_record, lock_record, status, live)

	cursor.execute(query_insert_DeviceType, args)
	db.commit()

def insert_DS(db, cursor, new_data):
	query_insert_DeviceStatus = "INSERT INTO DeviceStatus(MACAdd, CUR_Time, CUR_Position, CUR_Status) VALUES(%s, %s, %s, %s)"

	macadd = str(new_data[0])
	time = str(new_data[1])
	#time.strftime('%Y-%m-%d %H:%M:%s')
	position = new_data[2]
	#print(position)
	status = str(WHITE)
	args = (macadd, time, position, status, )

	cursor.execute(query_insert_DeviceStatus, args)
	db.commit()


def update_status(db, cursor, new_data, previous_data):
	""" define element """
	macadd = new_data[0]

	time_cur = new_data[1]
	position_cur = new_data[2]

	#print("position_current:", position_cur)

	time_pre = previous_data[1]
	statictime_pre = previous_data[2]
	position_pre = previous_data[3]
	del_pre = previous_data[4]
	lock_pre = previous_data[5]
	status_pre = previous_data[6]
	live_pre = previous_data[7]

	""" Values for check ~ should be modifiec later """
	DELETE_NUM = 2880
	T1 = 60
	T2 = 60
	T3 = 60
	LOCK_NUM  = 6

	""" --------------------- check whether LIVE ---------------------- """

	# Status: The device is NOT IN the room BEFORE
	if live_pre == 0:

		# Status: The device is also NOT IN the room NOW				
		if position_cur == None:

			# Status: The device is NOT IN the room for SOME TIME			
			if del_pre < DELETE_NUM:
				del_update = str(del_pre + 1)

				args = (del_update, macadd,)

				cursor.execute("UPDATE DeviceType SET Del_R = %s WHERE MACAdd = %s", args)
				db.commit()

				record_status(db, cursor, macadd, time_cur, position_pre, status_pre, time_pre, position_pre, status_pre)

			# Status: The device is NOT IN the room for LONG TIME
			else:							
				cursor.execute("DELETE FROM DeviceType WHERE MACAdd = %s", (macadd,))
				db.commit()

				record_status(db, cursor, macadd, NULL, NULL, NULL, NULL, NULL, NULL)


		# Status: The device ENTER the room
		else:						
			time_update = time_cur
			time_update.strftime('%Y-%m-%d %H:%M:%s')
			position_update = str(position_cur)
			live_update = str(1)
			del_update = str(0)

			args = (time_update, position_update, del_update, live_update, macadd, )

			cursor.execute("UPDATE DeviceType SET Time=%s,Position=%s,Del_R=%s,LIVE=%s WHERE MACAdd=%s", args)
			db.commit()

			record_status(db, cursor, macadd, time_update, position_update, status_pre, time_pre, position_pre, status_pre)

	# Status: The device is in the room before
	else:
		# Status: The device LEAVE room
		if position_cur == None:
			time_update = time_cur
			time_update.strftime('%Y-%m-%d %H:%M:%s')
			static_T_update = str(0) 		# more discussion
			position_update = None
			delete_R_update = str(del_pre + 1)
			live_update = str(0)

			args = (time_update, static_T_update, position_update, delete_R_update, live_update, macadd, )

			cursor.execute("UPDATE DeviceType SET TIME=%s,StaticTime=%s,Position=%s,Del_R=%s,LIVE=%s WHERE MACAdd=%s", args)
			db.commit()

			record_status(db, cursor, macadd, time_update, position_update, status_pre, time_pre, position_pre, status_pre)

		# Status: The device is IN the room BEFORE and NOW
		else:

			# Status: The device is IN the room and STATIC
			if position_cur == position_pre:

				# Status: Previous status is WHITE & Now STATIC
				if status_pre == WHITE:
					t_diff = time_diff(time_pre, time_cur)
					#print(t_diff)

					# Status: The device is in WHITE. & static time less than T1.
					if t_diff < T1:   # T1 need to be defined in previous part
						statictime_update = str(t_diff)

						args = (t_diff, macadd, )

						cursor.execute("UPDATE DeviceType SET StaticTime=%s WHERE MACAdd=%s", args)
						db.commit()

						record_status(db, cursor, macadd, time_cur, position_pre, status_pre, time_pre, position_pre, status_pre)

					# Status: The device is WHITE. & static time more than T1. Transfer to next mode (GRAY)
					else:
						time_update = time_cur
						time_update.strftime('%Y-%m-%d %H:%M:%s')
						statictime_update = str(0)
						status_update = str(2)

						args = (time_update, statictime_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s transform from WHITE mode to GRAY mode" % macadd)

						record_status(db, cursor, macadd, time_update, position_pre, status_update, time_pre, position_pre, status_pre)


				# Status: Previous status is GRAY & Now STATIC
				elif status_pre == 2:
					t_diff = time_diff(time_pre, time_cur)
					T2 = T2 - 10 * lock_pre;

					# Status: The device is in GRAY. & static time less than T2.
					if t_diff < T2:
						statictime_update = str(t_diff)

						args = (t_diff, macadd, )

						cursor.execute("UPDATE DeviceType SET StaticTime=%s WHERE MACAdd=%s", args)
						db.commit()

						record_status(db, cursor, macadd, time_cur, position_pre, status_pre, time_pre, position_pre, status_pre)

					# Status: The device is GRAY. & static time more than T2. Transfer to next mode (BLACK)
					else:
						time_update = time_cur
						time_update.strftime('%Y-%m-%d %H:%M:%s')
						statictime_update = str(0)
						status_update = str(BLACK)

						args = (time_update, statictime_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s transform from GRAY mode to Black mode" % macadd)

						record_status(db, cursor, macadd, time_update, position_pre, status_update, time_pre, position_pre, status_pre)


				# Status: Previous status is BLACK & Now STATIC
				elif status_pre == BLACK:
					t_diff = time_diff(time_pre, time_cur)

					# Status: The device is in BLACK. & static time less than T3.
					if t_diff < T3:
						statictime_update = str(t_diff)

						args = (t_diff, macadd, )

						cursor.execute("UPDATE DeviceType SET StaticTime=%s WHERE MACAdd=%s", args)
						db.commit()

						record_status(db, cursor, macadd, time_cur, position_pre, status_pre, time_pre, position_pre, status_pre)
					
					# Status: The device is BLACK. & static time more than T2. Transfer to next mode (LOCK)
					else:
						time_update = time_cur
						time_update.strftime('%Y-%m-%d %H:%M:%s')
						statictime_update = None
						status_update = str(0)

						args = (time_update, statictime_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s transform from BLACK mode to LOCK mode" % macadd)	

						record_status(db, cursor, macadd, time_update, position_pre, status_update, time_pre, position_pre, status_pre)

				# Status: Previous status is LOCK & Now STATIC
				elif status_pre == 0:
					pass

			# Status: The device is IN the room and MOVE
			else:
				# Status: Previous status is WHITE & Now MOVE
				if status_pre == WHITE:
					time_update = time_cur
					time_update.strftime('%Y-%m-%d %H:%M:%s')
					statictime_update = str(0)
					position_update = str(position_cur)

					args = (time_update, statictime_update, position_update, macadd, )

					cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, Position=%s WHERE MACAdd=%s", args)
					db.commit()

					record_status(db, cursor, macadd, time_update, position_update, status_pre, time_pre, position_pre, status_pre)

				# Status: Previous status is GRAY & Now MOVE & Transfer back to WHITE 
				elif status_pre == 2:
					if lock_pre == 0:
						time_update = time_cur
						time_update.strftime('%Y-%m-%d %H:%M:%s')
						statictime_update = str(0)
						position_update = str(position_cur)
						status_update = str(WHITE)

						args = (time_update, statictime_update, position_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, Position=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s transform from GRAY to WHITE" % macadd)

						record_status(db, cursor, macadd, time_update, position_update, status_update, time_pre, position_pre, status_pre)
					else:
						pass
				# Status: Previous status is BLACK & Now MOVE
				elif status_pre == BLACK:
					# Status: Transformation TIMES between GRAY and BLACK < M
					if lock_pre < LOCK_NUM:
						time_update = time_cur
						time_update.strftime('%Y-%m-%d %H:%M:%s')
						statictime_update = str(0)
						position_update = str(position_cur)
						lock_update = str(lock_pre + 1)
						status_update =  str(2)

						args = (time_update, statictime_update, position_update, lock_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, Position=%s, Lock_R=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s tranform from BLACK mode to GRAY mode" % macadd)

						record_status(db, cursor, macadd, time_update, position_update, status_update, time_pre, position_pre, status_pre)
					
					# Status: Transformation TIMES between GRAY and BLACK > M & then LOCKED
					else:
						time_update = time_cur
						statictime_update = None
						status_update = str(0)

						args = (time_update, statictime_update, status_update, macadd, )

						cursor.execute("UPDATE DeviceType SET Time=%s, StaticTime=%s, STATUS=%s WHERE MACAdd=%s", args)
						db.commit()

						print("The device %s tranform from BLACK mode to LOCK mode" % macadd)

						record_status(db, cursor, macadd, time_update, position_pre, status_update, time_pre, position_pre, status_pre)
				# Status: Previous status is LOCK & Now MOVE
				elif status_pre == 0:
					pass

	

def time_diff(time_pre, time_cur):
	time_pre.strftime('%Y-%m-%d %H:%M:%s')
	time_cur.strftime('%Y-%m-%d %H:%M:%s')
	#t_diff = time_cur - time_pre
	time_pre_ts = time.mktime(time_pre.timetuple())
	time_cur_ts = time.mktime(time_cur.timetuple())

	t_diff = int(time_cur_ts - time_pre_ts) / 60
	#print(t_diff)
	return t_diff

def record_status(db, cursor, cur_mac, cur_time, cur_position, cur_status, pre_time, pre_position, pre_status):
	query_record_status = "UPDATE DeviceStatus SET CUR_Time=%s,CUR_Position=%s,CUR_Status=%s,PRE_Time=%s,PRE_Position=%s,PRE_Status=%s WHERE MACAdd=%s"
	args = (cur_time, cur_position, cur_status, pre_time, pre_position, pre_status, cur_mac, )

	cursor.execute(query_record_status, args)
	db.commit()

	print("Finished updating Device Status")

def clear_tables(db, cursor):
	query_select_clear_mac = "SELECT MACAdd from DeviceStatus WHERE PRE_Time is NULL"
	mac_to_del = []

	cursor.execute(query_select_clear_mac)

	row = cursor.fetchone()

	while row is not None:
		#print(row)
		mac_to_del.append(row)
		row = cursor.fetchone()

	#print(mac_to_del)

	clear_deviceType(db, cursor, mac_to_del)

	clear_deviceStatus(db, cursor)

def clear_deviceType(db, cursor, mac_to_del):
	query_clear_devicetype = "DELETE FROM DeviceType WHERE MACAdd = %s"

	length_mac = len(mac_to_del)

	for i in range(0, length_mac):
		del_macadd = mac_to_del[i][0]
		#print(del_macadd)
		cursor.execute(query_clear_devicetype, (del_macadd, ))
		db.commit()
		print("Finish Moving MAC Address %s from DeviceType" % del_macadd)

def clear_deviceStatus(db, cursor):
	query_clear_devicestatus = "DELETE FROM DeviceStatus WHERE PRE_Time is NULL"

	cursor.execute(query_clear_devicestatus)
	db.commit()
	print("Finish Clear DeviceStatus Table")




	

