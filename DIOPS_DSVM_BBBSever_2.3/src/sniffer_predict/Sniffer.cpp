//
//  Sniffer.cpp
//  sniffer
//
//  Created by yuehust on 13/3/14.
//  Copyright (c) 2014 yuehust. All rights reserved.
//
/* 2015-01-18 new change for signal filter
 * RSSI column: signed char -> double
 * RSSI_all & RSSI_num: record all captured packets for signal filter 
 * 
 * ALTER TABLE MACRecord 
 * MODIFY COLUMN RSSI1 DECIMAL(5,2), 
 * MODIFY COLUMN RSSI2 DECIMAL(5,2), 
 * MODIFY COLUMN RSSI3 DECIMAL(5,2), 
 * MODIFY COLUMN RSSI4 DECIMAL(5,2), 
 * MODIFY COLUMN RSSI5 DECIMAL(5,2);
 */

#include "Sniffer.h"

unsigned char   MAC_add[MAX_COUNT_NUM][6];
//signed char     RSSI[MAX_COUNT_NUM];
unsigned long   MAC_count;

MYSQL*	conn;
unsigned long	DB_lines;
unsigned char	DBMAC[MAX_COUNT_NUM][6];
//signed char		DBRSSI[MAX_COUNT_NUM];
unsigned char	DBRecord[MAX_COUNT_NUM];

unsigned long	DB_static_lines;
unsigned long	static_rows;
unsigned char	StaticMAC[MAX_COUNT_NUM][6];
//signed char		StaticRSSI[MAX_COUNT_NUM];

//added by cyh 2015-01-18 for signal filter
double			RSSI[MAX_COUNT_NUM];
double			DBRSSI[MAX_COUNT_NUM];
double			StaticRSSI[MAX_COUNT_NUM];
double     		RSSI_all[MAX_COUNT_NUM][SNIFFER_NUM];
unsigned char	RSSI_num[MAX_COUNT_NUM];

//replace the defination
int AP_NO;

// added by cyh 2015-07-28 for easier modification when IP and AP_NUM changes
// these vars can be set from input (argc, argv)
char* SERVERIP;
int AP_NUM;

Sniffer::Sniffer()
{
	MAC_count   	= 0;
	DB_lines		= 0;
	static_rows		= 0;
	DB_static_lines	= 0;
	
	memset(MAC_add,0,sizeof(MAC_add));
	memset(RSSI,0,sizeof(RSSI));
	memset(DBMAC,0,sizeof(DBMAC));
	memset(DBRSSI,0,sizeof(DBRSSI));
	memset(DBRecord,0,sizeof(DBRecord));
	memset(StaticMAC,0,sizeof(StaticMAC));
	memset(StaticRSSI,0,sizeof(StaticRSSI));
	//added by cyh 2015-01-18 for signal filter
	memset(RSSI_all,0,sizeof(RSSI_all));
	memset(RSSI_num,0,sizeof(RSSI_num));
}

Sniffer::~Sniffer()
{

}

/*----------------Parse args from input------------------------------*/
/*----------------rows,columns,server ip, train ip, train mac--------*/
void Sniffer::ParseArguments(int argc, char **argv) {

	// default values
	AP_NUM = 5;
	SERVERIP = (char*) "192.168.1.189";

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (++i >= argc)
			exit(-1);
		switch (argv[i - 1][1]) {
		case 'n':	// number of APs
			AP_NUM = atoi(argv[i]);
			break;
		case 's':	// server ip (database)
			//memset(SERVERIP, 0, 50); how to initialize?
			SERVERIP = argv[i];
			break;
		default:
			fprintf(stderr, "Unknown option: -%c\n", argv[i - 1][1]);
			exit(-1);
			break;
		}
	}
}

/*----------------Set Wireless Card Monitor Mode-----------------------*/
void Sniffer::SetMode()
{
	char wlan_down[50]		= "sudo ifconfig ";
	char down[]				= " down";
	strcat(wlan_down, SNIFFER_DEVICE);
	strcat(wlan_down, down);
	
	char wlan_monitor[50]	= "sudo iwconfig ";
	char monitor[]			= " mode monitor";
	strcat(wlan_monitor, SNIFFER_DEVICE);
	strcat(wlan_monitor, monitor);
	
	char wlan_up[50]		= "sudo ifconfig ";
	char up[]				= " up";
	strcat(wlan_up, SNIFFER_DEVICE);
	strcat(wlan_up, up);

	//system("sudo ifconfig wlan0 down");
	//system("sudo iwconfig wlan0 mode monitor");
	//system("sudo ifconfig wlan0 up");
	system(wlan_down);
	system(wlan_monitor);
	system(wlan_up);
}

/*---------------Connect the Database----------------------------------*/
void Sniffer::DatabaseConn()
{
	if (mysql_library_init(0, NULL, NULL))
	{
	    fprintf(stderr, "could not initialize MySQL library\n");
	    exit(1);
	}

	conn = mysql_init(NULL);

	//if (!mysql_real_connect(conn,"localhost", "root", "root", "HAS",0,NULL,0))
	if (!mysql_real_connect(conn,SERVERIP, "root", "root", "HAS",0,NULL,0))
	{
		printf( "Error connecting to database: %s\n",mysql_error(conn));
	}
	else
	{
		printf("MySQL Connected...\n");
	}
}

/*---------------Read Data From Database-------------------------------*/
/*---------------Store All Data in array-------------------------------*/
/*---------------DBMAC DBRSSI DBRecord DB_lines------------------------*/ 
//select * from MACRecord where  RSSI3 IS NOT NULL
void Sniffer::DbGetMac()
{
	MYSQL_RES *store_result;
	MYSQL_ROW row;
	
	int query_result;

	char query[255]				= "select * from MACRecord where ";
	char write_rssi_string[]	= " RSSI";
	char write_notnull[]		= " IS NOT NULL";
	char write_ap_no[3];
	int ap_to_collect	= AP_NO;
	snprintf(write_ap_no, 3, "%d", ap_to_collect);
	
	strcat(query, write_rssi_string);
	strcat(query, write_ap_no);
	strcat(query, write_notnull);
	
	query_result = mysql_real_query(conn,query,(unsigned int) strlen(query));
 	if (query_result)
	{
		printf("Error making query: %s\n",
		mysql_error(conn));
	}
	
	store_result = mysql_store_result(conn);
	
	// added by cyh 2015-06-28
	if(store_result == NULL){
		printf("no data from mysql\n");
		return;
	}
	
 	while((row = mysql_fetch_row(store_result)) != 0)
	{
		for(int i=0;i<6;i++)
		{
			int step = 3*i;
			char tmp[3];
			strncpy(tmp,row[0]+step,2);
			DBMAC[DB_lines][i] =strtol(tmp,NULL,16);
		}
		DBRecord[DB_lines] 	= atoi(row[2]);
		//modified by cyh 2015-01-18 for signal filter
		//DBRSSI[DB_lines]	= atoi(row[AP_NO+2]);
		DBRSSI[DB_lines]	= atof(row[AP_NO+2]);
		
		if(DBRecord[DB_lines] == (STATIC_TIME_LIMIT+1))
		{
			for(int i=0;i<6;i++)
			{
				StaticMAC[static_rows][i]	= DBMAC[DB_lines][i];
			}
			StaticRSSI[static_rows]			= DBRSSI[DB_lines];
			//Sniffer::PrintStatic();
			static_rows++;
		}

		DB_lines++;
	}
	
 	// Store the DB static MAC Num
 	DB_static_lines	= static_rows;

	mysql_free_result(store_result);
}

/*---------------Update Database---------------------------------------*/
void Sniffer::DbUpdate()
{
	for(unsigned int i = DB_static_lines; i < static_rows; i++)
	{
		// revised by xhang 2016-01-08 [add the contraint 'only store the RSSI when it is more than -75']
		// if(StaticRSSI[i] > -75)
			Sniffer::DbAddMac(StaticMAC[i], StaticRSSI[i], STATIC_TIME_LIMIT+1);
	}
	for(unsigned long getline = 0; getline < MAC_count; getline++)
	{
		for(unsigned long dbline = 0; dbline < DB_lines; dbline++)
		{
			/*--------------changed by cyh 2014-12-29--------------------*/
			if(Sniffer::ExcludePackage(MAC_add[getline], DBMAC[dbline]) && DBRecord[dbline] > STATIC_TIME_LIMIT)
			{
				break;
			}

//			// DBMAC is the record stored in database
//			if(Sniffer::ExcludePackage(MAC_add[getline], DBMAC[dbline]))
//			{
//				// Record > 25 AP or Static one
//				if(DBRecord[dbline] > STATIC_TIME_LIMIT)
//				{
//					break;
//				}
//
//				//RSSI have changed, which means the location changed
//				//Update database
//				//Record add to array
//				else if((DBRSSI[dbline]-RSSI[getline] > 10) || (DBRSSI[dbline]-RSSI[getline] < -10))
//				{
//					Sniffer::DbAddMac(MAC_add[getline], RSSI[getline], 1);
//					//Sniffer::DbUpdateRssi(MAC_add[getline], RSSI[getline]);
//					break;
//				}
//				// If time condition meet, update database
//				// If not, just add to array
//				else
//				{
//					Sniffer::DbUpdateRecord(MAC_add[getline]);
//					break;
//				}
//			}
			// DBMAC is not in the database
			if(dbline == DB_lines-1)
			{
				//printf("     MACAdd: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", MAC_add[getline][0],MAC_add[getline][1],MAC_add[getline][2],MAC_add[getline][3],MAC_add[getline][4],MAC_add[getline][5]);
				// revised by xhang 2016-01-08 [add the contraint 'only store the RSSI when it is more than -75']
				// if(RSSI[getline] > -75)
					Sniffer::DbAddMac(MAC_add[getline], RSSI[getline], 1);
			}
		}
	}
}

/*--------------------Send Data into ServerGate------------------------*/
/*--------------------Socket Programming-------------------------------*/
void Sniffer::SendMAC()
{
	struct sockaddr_in serv_addr;
	char buffer[256];
	int sockfd;
	int status;
	char to_server[13];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("Error opening socket\n");
		exit(0);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serv_addr.sin_addr);
	serv_addr.sin_port = htons(SERVERPORT);
	
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error connecting\n");
		exit(0);
	}

	for(unsigned int i = 0; i < MAC_count; i++)
	{
		bzero(to_server,13);
		bzero(buffer, 256);
		to_server[0]	= 'M';
		to_server[1]	= 'A';
		to_server[2]	= 'C';
		for(int j = 3; j < 9; j++)
		{
			to_server[j]	= MAC_add[i][j-3];
		}
		to_server[9]	= '_';
		// the following code needs modified 2015-01-18
		to_server[10]	= RSSI[i];
		to_server[11]	= '_';
		to_server[12]	= '\n';
		to_server[13]	= '\0';
		
		status	= write(sockfd, to_server, 13);
		if(status < 0)
		{
			perror("Error writing to server");
			exit(0);
		}
		
		status	= read(sockfd, buffer, 255);
		if(status < 0)
		{
			perror("Error reading from server");
			exit(0);
		}
		//printf("%s", buffer);
	}
	close(sockfd);
}

/*--------------------Add New Data into Database-----------------------*/
/*--------------------add_mac add_rssi record(args)--------------------*/
/* insert into MACRecord (MACAdd,  RSSI1, Record)
 * values("18-00-2D-3D-99-8E",-85,1)
 * ON DUPLICATE KEY UPDATE Position = NULL,
 * RSSI1 = VALUES( RSSI1),
 * Record = IF(Record = 25, 25, VALUES(Record)),
 * RSSI2 = IF(timestampdiff(second,Timestamp,now()) < 2 ,  RSSI2 ,NULL)   */
//modified by cyh 2015-01-18 for signal filter
//void Sniffer::DbAddMac(const unsigned char add_mac[6], const signed char add_rssi, int record)
void Sniffer::DbAddMac(const unsigned char add_mac[6], const double add_rssi, int record)
{
	char query[1024]				= "insert into MACRecord (MACAdd, ";
	char write_rssi_string[]	= " RSSI";
	char write_value_string[]	= " = VALUES(";
	char write_bridge[]			= ":";
	char write_par[]			= ")";
	char write_comma[]			= ",";
	char write_quot[]			= "\"";
	char write_RSSI[10];
	char write_record[10];
	char write_ap_no[3];
	char write_time_diff[3];


	snprintf(write_record, 10, "%u", record);


	char write_values[]			= ", Record) values(\"";
	//char write_duplicate[]		= " ON DUPLICATE KEY UPDATE Position = NULL, ";
	char write_duplicate[]		= " ON DUPLICATE KEY UPDATE ";
	char write_record_update[]	= "), Record = IF(Record = 25, 25, VALUES(Record))";
	int ap_to_collect	= AP_NO;
	snprintf(write_ap_no, 3, "%d", ap_to_collect);

	strcat(query, write_rssi_string);	//insert into MACRecord (MACAdd, RSSI
	strcat(query, write_ap_no);			//insert into MACRecord (MACAdd, RSSI1
	strcat(query, write_values);		//insert into MACRecord (MACAdd, RSSI1, , Record) values(\"
	
	//revised by xhang 2016.1.8
	snprintf(write_RSSI, 10, "%5.2f", add_rssi);
	//if(add_rssi > -75)
	//	snprintf(write_RSSI, 10, "%5.2f", add_rssi);
	//else
	//	snprintf(write_RSSI, 10, "NULL");
			
	
	for(int i = 0; i < 6; i++)
	{
		char write_mac[3];
		//char write_colon[]	= ":";
		snprintf(write_mac, 3, "%.2X", add_mac[i]);	
		strcat(query, write_mac);
		if(i < 5)
		{
			//strcat(query, write_colon);
			strcat(query, write_bridge);
		}
	}

	strcat(query, write_quot);
	strcat(query, write_comma);
	strcat(query, write_RSSI);
	strcat(query, write_comma);
	strcat(query, write_record);
	strcat(query, write_par);
	strcat(query, write_duplicate);
	strcat(query, write_rssi_string);
	strcat(query, write_ap_no);
	strcat(query, write_value_string);
	strcat(query, write_rssi_string);
	strcat(query, write_ap_no);
	strcat(query, write_record_update);

	int time_diff	= GATES_TIME_DIFF;
	snprintf(write_time_diff, 3, "%d", time_diff);

	for (int i = 1; i <= AP_NUM; i++)
	{
		char write_if_time[]		= " = IF(timestampdiff(second,Timestamp,now()) < ";
		char write_default_rssi[]	= " ,NULL)";
		snprintf(write_ap_no, 3, "%d", i);
		if(i == AP_NO)
		{
			continue;
		}
		else
		{
			strcat(query, write_comma);
			strcat(query, write_rssi_string);
			strcat(query, write_ap_no);
			strcat(query, write_if_time);
			strcat(query, write_time_diff);
			strcat(query, write_comma);
			strcat(query, write_rssi_string);
			strcat(query, write_ap_no);
			strcat(query, write_default_rssi);
		}
/*
		if(i < AP_NUM-1)
		{
			strcat(query, write_comma);
		}
*/
	}
	//printf("%s\n",query);
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

/*----------If ∆RSII > certain threshold ------------------------------*/
/*----------Replace the Old RSSI With New One--------------------------*/
/*----------Set Record = 1 --------------------------------------------*/
/* update MACRecord set Position = NULL, RSSI1 = -85,Record = 1
 * where MACAdd = "48-60-BC-E7-9C-B2"                                  */
//modified by cyh 2015-01-18 for signal filter
//void Sniffer::DbUpdateRssi(const unsigned char* add_mac, const signed char add_rssi)
void Sniffer::DbUpdateRssi(const unsigned char* add_mac, const float add_rssi)
{
	char query[255]				= "update MACRecord set Position = NULL, ";
	char write_equal[]			= " = ";
	char write_rssi_string[]	= " RSSI";
	char write_comma[]			= ",";
	char write_record_set[]		= "Record = ";
	char write_record[]			= "1";
	char write_where[]			= " where MACAdd = \"";
	char write_quot[]			= "\"";

	char write_RSSI[10];
	char write_ap_no[3];

	int ap_to_collect	= AP_NO;
	snprintf(write_ap_no, 3, "%d", ap_to_collect);

	//modified by cyh 2015-01-18 for signal filter
	//snprintf(write_RSSI, 10, "%d", add_rssi);
	
	snprintf(write_RSSI, 10, "%5.2f", add_rssi);
	
	strcat(query, write_rssi_string);
	strcat(query, write_ap_no);
	strcat(query, write_equal);
	strcat(query, write_RSSI);
	strcat(query, write_comma);
	strcat(query, write_record_set);
	strcat(query, write_record);
	strcat(query, write_where);
	for(int i = 0; i < 6; i++)
	{
		char write_mac[3];
		//char write_colon[]	= ":";
		char write_bridge[]	= ":";
		snprintf(write_mac, 3, "%.2X", add_mac[i]);
		strcat(query, write_mac);
		if(i < 5)
		{
			//strcat(query, write_colon);
			strcat(query, write_bridge);
		}
	}
	strcat(query, write_quot);
	//printf("%s\n",query);
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

/*-------------When ∆Time > TIMECOUNT----------------------------------*/
/*-------------Record++------------------------------------------------*/
void Sniffer::DbUpdateRecord(const unsigned char* add_mac)
{
	char query[255]		= "update MACRecord set Record = Record+1";
	// ∆Hour > 1 hour, then record++
	char write_where[]	= " where timestampdiff(hour,Timestamp,NOW()) > 0 and MACAdd = \"";
	char write_quot[]	= "\"";
	strcat(query, write_where);
	for(int i = 0; i < 6; i++)
	{
		char write_mac[3];
		//char write_colon[]	= ":";
		char write_bridge[]	= ":";
		snprintf(write_mac, 3, "%.2X", add_mac[i]);
		strcat(query, write_mac);
		if(i < 5)
		{
			//strcat(query, write_colon);
			strcat(query, write_bridge);
		}
	}
	strcat(query, write_quot);
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

/*------------Delete MAC Record in Database----------------------------*/
void Sniffer::DbDeleteMac(const unsigned char* add_mac)
{
	char query[255]		= "delete from MACRecord where MACAdd = \"";
	char write_quot[]	= "\"";
	for(int i = 0; i < 6; i++)
	{
		char write_mac[3];
		//char write_colon[]	= ":";
		char write_bridge[]	= ":";
		snprintf(write_mac, 3, "%.2X", add_mac[i]);
		strcat(query, write_mac);
		if(i < 5)
		{
			//strcat(query, write_colon);
			strcat(query, write_bridge);
		}
	}
	strcat(query, write_quot);
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

/*-----------If Src is an AP-------------------------------------------*/
/*-----------Set Record = STATIC_TIME_LIMIT + 1 -----------------------*/
//modified by cyh 2015-01-18 for signal filter
//void Sniffer::SetRecordStatic(const unsigned char* add_mac, const signed char add_rssi)
void Sniffer::SetRecordStatic(const unsigned char* add_mac, const double add_rssi)
{
	MYSQL_RES *store_result;
	MYSQL_ROW row;

	int query_result;

	char query[255]	= "select Record from MACRecord where MACAdd = \"";
	char write_quot[]	= "\"";
	for(int i = 0; i < 6; i++)
	{
		char write_mac[3];
		//char write_colon[]	= ":";
		char write_bridge[]	= ":";
		snprintf(write_mac, 3, "%.2X", add_mac[i]);
		strcat(query, write_mac);
		if(i < 5)
		{
			//strcat(query, write_colon);
			strcat(query, write_bridge);
		}
	}
	strcat(query, write_quot);

	query_result = mysql_real_query(conn,query,(unsigned int) strlen(query));

 	if (query_result)
	{
		printf("Error making query: %s\n",
		mysql_error(conn));
	}

	store_result = mysql_store_result(conn);
 	if((row = mysql_fetch_row(store_result)) != 0)
	{
		if(atoi(row[0]) != (STATIC_TIME_LIMIT+1))
		{
			Sniffer::DeleteMacRecord(add_mac);
		}
	}

	mysql_free_result(store_result);

	Sniffer::DbDeleteMac(add_mac);
	Sniffer::DbAddMac(add_mac, add_rssi, STATIC_TIME_LIMIT + 1);
}

/*-----------Sort the Data Stored in Database--------------------------*/
/*-----------Delete Data too old---------------------------------------*/
/*-----------Release Database in case of unnecessary memory occupied---*/
// modified by cyh 2015-06-25

// void Sniffer::SortDBMAC(int interval_day, int record)
// {
// 	char query[255]	= "delete from MACRecord where date_sub(curdate(),interval ";
// 	char write_interval_day[4];
// 	char write_record[4];
// 	snprintf(write_interval_day, 4, "%u", interval_day);
// 	snprintf(write_record, 4, "%u", record+1);
// 	strcat(query, write_interval_day);
// 	char query_end[]	= " day) > Timestamp and Record < ";
// 	strcat(query,query_end);
// 	strcat(query,write_record);
// 	mysql_real_query(conn,query,(unsigned int) strlen(query));
// }
void Sniffer::SortDBMAC(int interval_minute, int record)
{
	char query[255]	= "delete from MACRecord where timestampdiff(minute,Timestamp, now()) > ";
	
	char write_interval_minute[4];
	char write_record[4];
	
	snprintf(write_interval_minute, 4, "%u", interval_minute);
	snprintf(write_record, 4, "%u", record+1);
	
	strcat(query, write_interval_minute);
	
	char query_end[]	= " and Record < ";
	
	strcat(query,query_end);
	strcat(query,write_record);
	
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

//modified by cyh 2015-01-18 for signal filter
//void Sniffer::AddStaticRecord(const unsigned char* add_mac, const signed char add_rssi)
void Sniffer::AddStaticRecord(const unsigned char* add_mac, const double add_rssi)
{
	// if exists in StaticMAC
	for(unsigned int i = 0; i < static_rows; i++)
	{
		if(Sniffer::ExcludePackage(add_mac ,StaticMAC[i]))
		{
			return;
		}
	}
	// if exists in MAC_add, delete the record in MAC_add
	for(unsigned int i = 0; i < MAC_count; i++)
	{
		if(Sniffer::ExcludePackage(add_mac ,MAC_add[i]))
		{
			Sniffer::DeleteMacRecord(MAC_add[i]);
		}
	}
	for(int i = 0; i < 6; i++)
	{
		StaticMAC[static_rows][i]	= add_mac[i];
	}
	StaticRSSI[static_rows]			= add_rssi;

	// add the record number
	//Sniffer::PrintStatic();
	static_rows++;
}

//modified by cyh 2015-01-18 for signal filter
//void Sniffer::AddNewMacRecord(const unsigned char* add_mac, const signed char add_rssi)
void Sniffer::AddNewMacRecord(const unsigned char* add_mac, const double add_rssi)
{
	// if exists in MAC_add
	for(unsigned int i = 0; i < MAC_count; i++)
	{
		if(Sniffer::ExcludePackage(add_mac ,MAC_add[i]))
		{
			//added by cyh 2015-01-18 for signal filter
			RSSI_num[i]++;
			RSSI_all[i][RSSI_num[i]-1]	= add_rssi;
			return;
		}
	}
	// if exists in StaticMAC
	for(unsigned int i = 0; i < static_rows; i++)
	{
		if(Sniffer::ExcludePackage(add_mac ,StaticMAC[i]))
		{
			return;
		}
	}
	for(int i=0;i<6;i++)
	{
		MAC_add[MAC_count][i]	= add_mac[i];
	}
	//modified by cyh 2015-01-18 for signal filter
	RSSI[MAC_count]				= add_rssi;
	//added by cyh 2015-01-18 for signal filter
	RSSI_num[MAC_count]			= 1;
	RSSI_all[MAC_count][RSSI_num[MAC_count]-1]	= add_rssi;

	//commented by cyh 2015-06025
	Sniffer::PrintResult();
	// add the record number
	MAC_count++;
}

void Sniffer::GetRssiDistribute()
{
	for(unsigned int i = 0; i < MAC_count; i++)
	{
		double tmp = 0;
		for(unsigned int j = 0; j < RSSI_num[i]; j++)
		{
			tmp += RSSI_all[i][j];
			//commented by cyh 2015-06-26
			//printf("%5.2f\t",RSSI_all[i][j]);
		}
		RSSI[i] = tmp/RSSI_num[i];
		//commented by cyh 2015-06-26
		//printf("RSSI:%5.2f\n",RSSI[i]);
	}
}

void Sniffer::DeleteMacRecord(const unsigned char* add_mac)
{
	for(unsigned long i = 0; i <= MAC_count; i++)
	{
		if(Sniffer::ExcludePackage(add_mac, MAC_add[i]))
		{
			if(MAC_count > (i+1))
			{
				for(unsigned long j = i+1; j <= MAC_count; j++)
				{
					for(int k = 0; k < 6; k++)
					{
						MAC_add[j-1][k]	= MAC_add[j][k];
					}
					RSSI[j-1]			= RSSI[j];
				}
			}
			else
			{
				for(int k = 0; k < 6; k++)
				{
					MAC_add[i][k]	= 0x00;
				}
				RSSI[i]				= 0x00;
			}
			MAC_count--;
			return;
		}
	}
}

/*-----------Get the Net Packets Sniffered from Wireless Card-------------*/
/*-----------Exclude Invalid MAC Address----------------------------------*/
void Sniffer::GetPacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	// declare pointers to packet headers
	const struct radiotap_header *head;	/* The raidotap header*/
	const struct wifi_header *data;		/* The wifi data header */
        
	// define raidotap header
	head = (struct radiotap_header*)(packet);
	
	// define/compute wifi data offset
	data = (struct wifi_header*)(packet + (head->hd_len[0]));

	/*------------------Invalid From Head Inform-----------------------*/
	// Struct not as defined:TSFT Flags Rate Channel dB_Antenna_Signal
	if((head->prst_flg[0] & 0x3f) != 0x2f)
	{
		return;
	}
	//.... 1... = Fragmentation: True
	if((head->flg[0] & 0x08) == 0x08)
	{
		return;
	}
	// ...1 .... = FCS at end: false
	if((head->flg[0] & 0x10) == 0x00)
	{
		return;
	}
	// .1.. .... = Bad FCS: True
	if((head->flg[0] & 0x40) == 0x40)
	{
		return;
	}

	/*------------------Invalid From Data Inform-----------------------*/
	// Reserved Type: 11 return
	if (((data->frame_ctrl[0]) & 0x0c) == 0x0c)
	{
		return;
	}
	// Reserved in Management Frames
	if (((data->frame_ctrl[0]) & 0x6c) == 0x60)
	{
		return;
	}
	// Reserved in Control Frames
	if (((data->frame_ctrl[0]) & 0x8c) == 0x04)
	{
		return;
	}
	// Reserved in Data Frames
	if (((data->frame_ctrl[0]) & 0xfc) == 0xd8)
	{
		return;
	}
	// CTS frame, no tx add, return
	if ((data->frame_ctrl[0] == 0xc4))
	{
		return;
	}
	// ACK frame, no tx add, return
	if ((data->frame_ctrl[0] == 0xd4))
	{
		return;
	}
	// Beacon frame/Reassociation request/Association response, tx is an AP
	if (((data->frame_ctrl[0]& 0xfc) == 0x80) || ((data->frame_ctrl[0]& 0xfc) == 0x30) || ((data->frame_ctrl[0]& 0xfc) == 0x10))
	{
		//SetRecordStatic(data->tx_add, head->ssi_sgn[0]);
		AddStaticRecord(data->tx_add, (double)head->ssi_sgn[0]);
		return;
	}
	// Type/Subtype: Null function (No data) (0x24)
	if ((data->frame_ctrl[0]& 0xfc) == 0x48)
	{
		return;
	}
	// Type/Subtype: ATIM (0x09)
	if ((data->frame_ctrl[0]& 0xfc) == 0x90)
	{
		return;
	}
	// To DS: 0 From DS: 0(Not leaving DS or network is operating in AD-HOC mode)
	if(((data->frame_ctrl[1]) & 0x03) == 0x00)
	{
		// Not probe request
		if(((data->frame_ctrl[0]) & 0xfc) != 0x40)
		{
			return;
		}
		//return;
	}
	// To DS: 1 From DS: 1(Frame part of WDS from one AP to another AP)
	if(((data->frame_ctrl[1]) & 0x03) == 0x03)
	{
		return;
	}
	// no valid rssi,return
	// revised by xhang 2016-01-08 [add contrains that return when ssi less than -75]
	// if (head->ssi_sgn[0] > 0)
	if (head->ssi_sgn[0] > 0 || head->ssi_sgn[0] < -80)
	{
		return;
	}

	Sniffer::AddNewMacRecord(data->tx_add, double(head->ssi_sgn[0]));

	return;
}

int Sniffer::ExcludePackage(const unsigned char capture_mac[6] ,const unsigned char exclude_mac[6])
{
	if(capture_mac[0]==exclude_mac[0] && capture_mac[1]==exclude_mac[1] && capture_mac[2]==exclude_mac[2] && capture_mac[3]==exclude_mac[3] && capture_mac[4]==exclude_mac[4] && capture_mac[5]==exclude_mac[5])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void Sniffer::PrintResult()
{
	printf("\nPacket number %ld:\n", MAC_count+1);
	printf("     MACAdd: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", MAC_add[MAC_count][0],MAC_add[MAC_count][1],MAC_add[MAC_count][2],MAC_add[MAC_count][3],MAC_add[MAC_count][4],MAC_add[MAC_count][5]);
	printf("       RSSI: %5.2f\n", RSSI[MAC_count]);
}

void Sniffer::PrintStatic()
{
	printf("\nStatic number %ld:\n", static_rows+1);
	printf("     MACAdd: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", StaticMAC[static_rows][0],StaticMAC[static_rows][1],StaticMAC[static_rows][2],StaticMAC[static_rows][3],StaticMAC[static_rows][4],StaticMAC[static_rows][5]);
	printf("       RSSI: %5.2f\n", StaticRSSI[static_rows]);
}
void Sniffer::NetSniffer()
{
//	SetMode();

//	DatabaseConn();

	/* Update message for this version: 
	* 1. remove pcap_activate(handle), because it is included in the function pcap_open_live().
	* 2. move pcap_open_live() out of while(1) loop. For some reason, the exection will stop at pcap_activate(handle) or pcap_open_live().
	* 3. if interval 1 day and delete records -> if interval 10 minutes and delete records.
	*/
	handle = pcap_open_live(SNIFFER_DEVICE, 70, 1, 500, errbuf);
	if(handle == NULL)
	{
		printf("pcap_create failed: %s\n", errbuf);
		exit(EXIT_FAILURE);
	}
	if(pcap_setnonblock(handle, 1, errbuf) < 0)
	{
		printf("pcap_setnonblocking error:%s\n", errbuf);
		exit(EXIT_FAILURE);
	}
	//printf("link-type: %d\n", pcap_datalink(handle));
	//printf("link-type: %s\n", pcap_datalink_val_to_name(pcap_datalink(handle)));
	
	/*---------------------Second Parameter, 70, Set the snapshot length to 70(34+?<70)---------------------------*/
	/*---------------------Third Parameter, 1, Turn promiscuous mode on-------------------------------------------*/
	/*---------------------Forth Parameter, 500, Set the timeout to 512 milliseconds------------------------------*/
	while(1)
	{
//		time_t begin,end;
//		begin	= clock();

		Sniffer();
		
		SortDBMAC(INTERVAL_MINUTE, STATIC_TIME_LIMIT);
		
		DbGetMac();
		
		int loop = pcap_dispatch(handle, SNIFFER_NUM, GetPacket, NULL);
        	if(loop == -1)
        	{
             	printf("Sniffer Error Occurs");
             	continue;
			}

        GetRssiDistribute();

		printf("\nTotal Packet number: %ld\n", MAC_count);
//		DbGetMac();

		DbUpdate();
		
//		end		= clock();
		
		//commented by cyh 2015-06-25
		//cout << "runtime: " << double(end-begin)/CLOCKS_PER_SEC*100 << " s\n" << endl;
		
		//SendMAC();
	}
	
	pcap_close(handle);
	
//	mysql_close(conn);
//	mysql_library_end();
}

void* SnifferRun(void* argument)
{
	//parse arguement from main.cpp
    arguments *arg	=(arguments *)argument;
    int argc	= arg->arg_char;
    char **argv	= arg->arg_vetcor;
	
	//get AP_NO from hostname
	//for example: hostname: board1 -> AP_NO=1
	char hostname[1024];
	gethostname(hostname,1024);
	char no_in_string[10];

	int j = 0;
	for(int i=0;i<sizeof(hostname);i++){
		if(isdigit(hostname[i])){
			no_in_string[j] = hostname[i];
			j++;
		}
	}
	AP_NO = atoi(no_in_string);	//printf("%d\n",AP_NO);
	
	Sniffer snif;

	snif.ParseArguments(argc, argv);
	
	snif.SetMode();
	snif. DatabaseConn();
	
	snif.NetSniffer();

	mysql_close(conn);
	mysql_library_end();

	return 0;
}
