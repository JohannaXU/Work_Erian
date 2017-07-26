//
//  sniffer.h
//  sniffer
//
//  Created by yuehust on 13/3/14.
//  Copyright (c) 2014 yuehust. All rights reserved.
//

#ifndef SNIFFER_H_
#define SNIFFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <mysql/mysql.h>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */ // For calculating time
#include <sys/unistd.h>

// #ifndef SERVERIP
// #define SERVERIP "192.168.1.189"
// #endif

// #ifndef TRAINIP
// #define TRAINIP "192.168.1.112"
// #endif

#ifndef SERVERPORT
#define SERVERPORT 1972
#endif

#ifndef SOCKETPORT
#define SOCKETPORT 8123
#endif

#ifndef SNIFFER_DEVICE
#define SNIFFER_DEVICE "wlan0"
#endif

#ifndef MAX_COUNT_NUM
#define MAX_COUNT_NUM 3000
#endif

#ifndef INTERVAL_DAY
#define INTERVAL_DAY 1
#endif

#ifndef STATIC_TIME_LIMIT
#define STATIC_TIME_LIMIT 24
#endif

#ifndef GATES_TIME_DIFF
#define GATES_TIME_DIFF 500000
#endif

#ifndef SNIFFER_NUM
#define SNIFFER_NUM 25000
#endif

// #ifndef AP_NUM
// #define AP_NUM 5
// #endif

// replace with a global number
// #ifndef AP_NO
// #define AP_NO 1
// #endif

#ifndef PACKETNUM
#define PACKETNUM 50
#endif

//#ifndef TRAIN_MAC
//#define TRAIN_MAC "38-EC-E4-6F-F5-B8"
//#endif

#ifndef CLOCK_PER_SEC
#define CLOCK_PER_SEC ((clock_t)10000)
#endif
/**********************ADDED*******************/
#ifndef PORT 
#define PORT 4446
#endif

#ifndef PORT1 
#define PORT1 4447
#endif

#ifndef GROUP 
#define GROUP "230.0.0.1"
#endif

#ifndef MSGBUFSIZE
#define MSGBUFSIZE 256
#endif
/**********************ADDED*******************/

using namespace std;

extern bool onCollecting;

struct radiotap_header
{
	unsigned char hd_rv[1];
	unsigned char hd_pad[1];
	unsigned char hd_len[2];
	unsigned char prst_flg[4];
	unsigned char mac_tstp[8];
	unsigned char flg[1];
	unsigned char dt_rt[1];
	unsigned char chnl_frq[2];
	unsigned char chnl_type[2];
	signed char ssi_sgn[1];
	unsigned char atn[1];
	unsigned char rx_flg[2];
};

struct wifi_header
{
	unsigned char frame_ctrl[2];
	unsigned char duration[2];
	unsigned char rx_add[6];
	unsigned char tx_add[6];
};

struct arguments
{
	int arg_char;
	char** arg_vetcor;
};

class Sniffer
{
private:
	pcap_t* handle;						/* Session handle */
	char    errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
	struct  pcap_pkthdr header;			/* The header that pcap gives us */
	const   u_char *packet;				/* The actual packet */

public:
	Sniffer();
	~Sniffer();
	void ParseArguments(int argc, char **argv);
	void SetDevice(const char* device);
	void SetMode();
	void DatabaseConn();
	// Operation in Database
	void DbGetMac();
	void SortDBMAC(int interval_day, int record);
	void DbUpdate();
	void SendMAC();
	void GetTrainMAC();
	int GetTrainID();
	int BcastSocket(const char* msgbuf, uint16_t port);
	static void DbAddMac(const unsigned char* add_mac, const signed char add_rssi, int record);
	static void DbUpdateRssi(const unsigned char* add_mac, const signed char add_rssi);
	static void DbUpdateRecord(const unsigned char* add_mac);
	static void DbDeleteMac(const unsigned char* add_mac);
	static void SetRecordStatic(const unsigned char* add_mac, const signed char add_rssi);
	// Operation in getting packages
	static void AddStaticRecord(const unsigned char* add_mac, const signed char add_rssi);
	static void AddNewMacRecord(const unsigned char* add_mac, const signed char add_rssi);
	static void DeleteMacRecord(const unsigned char* add_mac);
	static void GetPacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
	static int ExcludePackage(const unsigned char capture_mac[6] ,const unsigned char exclude_mac[6]);
	static void PrintResult();
	static void PrintStatic();
	void ClientSocket(const char* server_ip, const char* msg);
	void NetSniffer();
};


void* SnifferRun(void *argument);
#endif
