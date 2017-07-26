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
#include <sys/unistd.h>

// #ifndef SERVERIP
// #define SERVERIP "192.168.1.189"
// #endif

#ifndef SERVERPORT
#define SERVERPORT 1972
#endif

#ifndef SNIFFER_DEVICE
#define SNIFFER_DEVICE "wlan0"
#endif

#ifndef MAX_COUNT_NUM
#define MAX_COUNT_NUM 30000
#endif

#ifndef INTERVAL_MINUTE
#define INTERVAL_MINUTE 10
#endif

#ifndef STATIC_TIME_LIMIT
#define STATIC_TIME_LIMIT 24
#endif

#ifndef GATES_TIME_DIFF
#define GATES_TIME_DIFF 10
#endif

#ifndef SNIFFER_NUM
#define SNIFFER_NUM 300
#endif

// #ifndef AP_NUM
// #define AP_NUM 5
// #endif

// #ifndef AP_NO
// #define AP_NO 1
// #endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC ((clock_t)1000000)
#endif

using namespace std;

struct radiotap_header
{
	unsigned char hd_rv[1];
	unsigned char hd_pad[1];
	unsigned char hd_len[2];
//	unsigned char prst_flg[4];
	unsigned char prst_flg[8];
	unsigned char invalid_a[4];

	unsigned char mac_tstp[8];
	unsigned char flg[1];
	unsigned char dt_rt[1];
	unsigned char chnl_frq[2];
	unsigned char chnl_type[2];
	signed char ssi_sgn[1];

	unsigned char invalid_b[1];
	unsigned char rx_flg[2];
	signed char ssi_sgn_b[1];
	unsigned char atn[1];
//	unsigned char rx_flg[2];
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
	//modified by cyh 2015-01-18 for signal filter
	//static void DbAddMac(const unsigned char add_mac[6], const signed char add_rssi, int record);
	static void DbAddMac(const unsigned char add_mac[6], const double add_rssi, int record);
	
	//modified by cyh 2015-01-18 for signal filter
	//static void DbUpdateRssi(const unsigned char* add_mac, const signed char add_rssi);
	static void DbUpdateRssi(const unsigned char* add_mac, const float add_rssi);
	
	static void DbUpdateRecord(const unsigned char* add_mac);
	static void DbDeleteMac(const unsigned char* add_mac);
	
	//modified by cyh 2015-01-18 for signal filter
	//static void SetRecordStatic(const unsigned char* add_mac, const signed char add_rssi);
	static void SetRecordStatic(const unsigned char* add_mac, const double add_rssi);

	// Operation in getting packages
	//modified by cyh 2015-01-18 for signal filter
	//static void AddStaticRecord(const unsigned char* add_mac, const signed char add_rssi);
	static void AddStaticRecord(const unsigned char* add_mac, const double add_rssi);
	
	//modified by cyh 2015-01-18 for signal filter
	//static void AddNewMacRecord(const unsigned char* add_mac, const signed char add_rssi);
	static void AddNewMacRecord(const unsigned char* add_mac, const double add_rssi);
	
	static void DeleteMacRecord(const unsigned char* add_mac);
	static void GetPacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
	static int ExcludePackage(const unsigned char capture_mac[6] ,const unsigned char exclude_mac[6]);
	static void GetRssiDistribute();
	static void PrintResult();
	static void PrintStatic();
	void NetSniffer();
};


void* SnifferRun(void* arg);
#endif
