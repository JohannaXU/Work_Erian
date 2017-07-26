//
//  Sniffer.cpp
//  sniffer
//
//  Created by yuehust on 13/3/14.
//  Copyright (c) 2014 yuehust. All rights reserved.
//
//  23/10/2014 changed Runtime calculation
#include "Sniffer_train.h"

unsigned char MAC_add[MAX_COUNT_NUM][6];
signed char RSSI[MAX_COUNT_NUM];
unsigned long MAC_count;
unsigned char TrainMAC[6];

MYSQL* conn;

bool onCollecting;

int packCount;
unsigned long TrainID;
unsigned long TrainBlock;

// example: board1 -> AP_NO=1
int AP_NO;

// added by cyh 2015-07-28 for easier modification when IP and AP_NUM changes
// these vars can be set from input (argc, argv)
char* SERVERIP;
char* TRAINIP;
char* TRAIN_MAC;
int AP_NUM;
int ROW;
int COLUMN;

// initialization
Sniffer::Sniffer() {
	MAC_count = 0;

	memset(MAC_add, 0, sizeof(MAC_add));
	memset(RSSI, 0, sizeof(RSSI));

	memset(TrainMAC, 0, sizeof(TrainMAC));
}

Sniffer::~Sniffer() {

}

/*----------------Parse args from input------------------------------*/
/*----------------rows,columns,server ip, train ip, train mac--------*/
void Sniffer::ParseArguments(int argc, char **argv) {

	// default values
	ROW = 4;
	COLUMN = 12;
	AP_NUM = 5;
	SERVERIP = (char*) "192.168.1.189";
	TRAINIP = (char*) "192.168.1.112";
	TRAIN_MAC = (char*) "38-EC-E4-6F-F5-B8";

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (++i >= argc)
			exit(-1);
		switch (argv[i - 1][1]) {
		case 'r':	// rows
			ROW = atoi(argv[i]);
			break;
		case 'c':	// columns
			COLUMN = atoi(argv[i]);
			break;
		case 'n':	// number of APs
			AP_NUM = atoi(argv[i]);
			break;
		case 's':	// server ip (database)
			//memset(SERVERIP, 0, 50); how to initialize?
			SERVERIP = argv[i];
			break;
		case 't':	// train ip (pad for data collection)
			//memset(TRAINIP, 0, 50); how to initialize?
			TRAINIP = argv[i];
			break;
		case 'm':	// mac address (pad for data collection)
			//memset(TRAIN_MAC, 0, 50); how to initialize?
			TRAIN_MAC = argv[i];
			break;
		default:
			fprintf(stderr, "Unknown option: -%c\n", argv[i - 1][1]);
			exit(-1);
			break;
		}
	}
}

/*----------------Set Wireless Card Monitor Mode-----------------------*/
void Sniffer::SetMode() {
	char wlan_down[50] = "sudo ifconfig ";
	char down[] = " down";
	strcat(wlan_down, SNIFFER_DEVICE);
	strcat(wlan_down, down);

	char wlan_monitor[50] = "sudo iwconfig ";
	char monitor[] = " mode monitor";
	strcat(wlan_monitor, SNIFFER_DEVICE);
	strcat(wlan_monitor, monitor);

	char wlan_up[50] = "sudo ifconfig ";
	char up[] = " up";
	strcat(wlan_up, SNIFFER_DEVICE);
	strcat(wlan_up, up);

	system(wlan_down);		// system("sudo ifconfig wlan0 down");
	system(wlan_monitor);	// system("sudo iwconfig wlan0 mode monitor");
	system(wlan_up);		// system("sudo ifconfig wlan0 up");
}

// "38-EC-E4-6F-F5-B8" -> TrainMAC[0]~TrainMAC[5]
void Sniffer::GetTrainMAC() {
	for (int i = 0; i < 6; i++) {
		int step = 3 * i;
		char tmp[3];
		strncpy(tmp, TRAIN_MAC + step, 2);
		TrainMAC[i] = strtol(tmp, NULL, 16);
	}
}

/*---------------Connect the Database----------------------------------*/
void Sniffer::DatabaseConn() {
	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "could not initialize MySQL library\n");
		exit(1);
	}

	conn = mysql_init(NULL);

	//if (!mysql_real_connect(conn,"localhost", "root", "root", "HAS",0,NULL,0))
	if (!mysql_real_connect(conn, SERVERIP, "root", "root", "HAS", 0, NULL,
			0)) {
		printf("Error connecting to database: %s\n", mysql_error(conn));
	} else {
		printf("MySQL Connected...\n");
	}
}

/*--------------------Send Data into ServerGate------------------------*/
/*--------------------Socket Programming-------------------------------*/
/*--------------------No used in the code------------------------------*/
void Sniffer::SendMAC() {
	struct sockaddr_in serv_addr;
	char buffer[256];
	int sockfd;
	int status;
	char to_server[13];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Error opening socket\n");
		exit(0);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serv_addr.sin_addr);
	serv_addr.sin_port = htons(SERVERPORT);

	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		perror("Error connecting\n");
		exit(0);
	}

	for (unsigned int i = 0; i < MAC_count; i++) {
		bzero(to_server, 13);
		bzero(buffer, 256);
		to_server[0] = 'M';
		to_server[1] = 'A';
		to_server[2] = 'C';
		for (int j = 3; j < 9; j++) {
			to_server[j] = MAC_add[i][j - 3];
		}
		to_server[9] = '_';
		to_server[10] = RSSI[i];
		to_server[11] = '_';
		to_server[12] = '\n';
		to_server[13] = '\0';

		status = write(sockfd, to_server, 13);
		if (status < 0) {
			perror("Error writing to server");
			exit(0);
		}

		status = read(sockfd, buffer, 255);
		if (status < 0) {
			perror("Error reading from server");
			exit(0);
		}
		//printf("%s", buffer);
	}
	close(sockfd);
}

/*--------------------Add New Data into Database-----------------------*/
/*--------------------add_mac add_rssi record(args)--------------------*/
/* INSERT INTO MACTrain(Block,ID)
 * values(1,100001)
 * ON DUPLICATE KEY UPDATE RSSI3 = -10; */
void Sniffer::DbAddMac(const unsigned char add_mac[6],
		const signed char add_rssi, int record) {
	char query[255];
	snprintf(query, 255,
			"INSERT INTO MACTrain(Block,ID,RSSI%d) values(%lu,%lu,%d) ON DUPLICATE KEY UPDATE RSSI%d = %d",
			AP_NO, TrainBlock, TrainID, add_rssi, AP_NO, add_rssi);
	mysql_real_query(conn,query,(unsigned int) strlen(query));
}

/*-----------Get the Net Packets Sniffered from Wireless Card-------------*/
/*-----------Exclude Invalid MAC Address----------------------------------*/
void Sniffer::GetPacket(u_char *tx_mac_add, const struct pcap_pkthdr *header,
		const u_char *packet) {
	// declare pointers to packet headers
	const struct radiotap_header *head; /* The raidotap header*/
	const struct wifi_header *data; /* The wifi data header */
	
	// define raidotap header
	head = (struct radiotap_header*) (packet);

	// define/compute wifi data offset
	data = (struct wifi_header*) (packet + (head->hd_len[0]));

	/*------------------Invalid From Head Inform-----------------------*/
	// Struct not as defined:TSFT Flags Rate Channel dB_Antenna_Signal
	if ((head->prst_flg[0] & 0x3f) != 0x2f) {
		return;
	}
	//.... 1... = Fragmentation: True
	if ((head->flg[0] & 0x08) == 0x08) {
		return;
	}
	// ...1 .... = FCS at end: false
	if ((head->flg[0] & 0x10) == 0x00) {
		return;
	}
	// .1.. .... = Bad FCS: True
	if ((head->flg[0] & 0x40) == 0x40) {
		return;
	}

	/*------------------Invalid From Data Inform-----------------------*/
	// Reserved Type: 11 return
	if (((data->frame_ctrl[0]) & 0x0c) == 0x0c) {
		return;
	}
	// Reserved in Management Frames
	if (((data->frame_ctrl[0]) & 0x6c) == 0x60) {
		return;
	}
	// Reserved in Control Frames
	if (((data->frame_ctrl[0]) & 0x8c) == 0x04) {
		return;
	}
	// Reserved in Data Frames
	if (((data->frame_ctrl[0]) & 0xfc) == 0xd8) {
		return;
	}
	// CTS frame, no tx add, return
	if ((data->frame_ctrl[0] == 0xc4)) {
		return;
	}
	// ACK frame, no tx add, return
	if ((data->frame_ctrl[0] == 0xd4)) {
		return;
	}
	// Beacon frame/Reassociation request/Association response, tx is an AP
	if (((data->frame_ctrl[0] & 0xfc) == 0x80)
			|| ((data->frame_ctrl[0] & 0xfc) == 0x30)
			|| ((data->frame_ctrl[0] & 0xfc) == 0x10)) {
		return;
	}
	// Type/Subtype: Null function (No data) (0x24)
	if ((data->frame_ctrl[0] & 0xfc) == 0x48) {
		return;
	}
	// Type/Subtype: ATIM (0x09)
	if ((data->frame_ctrl[0] & 0xfc) == 0x90) {
		return;
	}
	// To DS: 0 From DS: 0(Not leaving DS or network is operating in AD-HOC mode)
	if (((data->frame_ctrl[1]) & 0x03) == 0x00) {
		// Not probe request
		if (((data->frame_ctrl[0]) & 0xfc) != 0x40) {
			return;
		}
		//return;
	}
	// To DS: 1 From DS: 1(Frame part of WDS from one AP to another AP)
	if (((data->frame_ctrl[1]) & 0x03) == 0x03) {
		return;
	}
	// no valid rssi,return
	// revised by xhang 2016-01-08 [add contrains that return when ssi less than -75]
	// if (head->ssi_sgn[0] > 0)
	if (head->ssi_sgn[0] > 0 || head->ssi_sgn[0] < -80)
	{
		return;
	}

	if (Sniffer::ExcludePackage(data->tx_add, TrainMAC)) {
		Sniffer::DbAddMac(TrainMAC, head->ssi_sgn[0], 1);
		printf("Record added: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", data->tx_add[0],
				data->tx_add[1], data->tx_add[2], data->tx_add[3],
				data->tx_add[4], data->tx_add[5]);
		printf("        RSSI: %d\n", head->ssi_sgn[0]);
		
		for (int i = 0; i < 6; i++) {
			tx_mac_add[i] = data->tx_add[i];
		}
		TrainID++;
		packCount++;
	}
	return;
}

int Sniffer::ExcludePackage(const unsigned char capture_mac[6],
		const unsigned char exclude_mac[6]) {
	if (capture_mac[0] == exclude_mac[0] && capture_mac[1] == exclude_mac[1]
			&& capture_mac[2] == exclude_mac[2]
			&& capture_mac[3] == exclude_mac[3]
			&& capture_mac[4] == exclude_mac[4]
			&& capture_mac[5] == exclude_mac[5]) {
		return 1;
	} else {
		return 0;
	}
}

void Sniffer::ClientSocket(const char* server_ip, const char* msg) {
	struct sockaddr_in servaddr;
	char buf[MSGBUFSIZE];
	int sockfd;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip, &servaddr.sin_addr);
	servaddr.sin_port = htons(SOCKETPORT);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int count = 0;
	loop: if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))
			< 0) {
		sleep(2);
		count++;
		if (count < 5) {
			goto loop;
		} else {
			perror("fail");
			return;
		}
	}

	write(sockfd, msg, strlen(msg));
	printf("Command Received from server:\n");

	if (strcmp(msg, "MODEL") == 0) {
		printf("\t\t\t\tSvm Model Training Finished!");
		printf("\n");
	} else if (strcmp(msg, "LABEL") == 0) {
		printf("\t\t\t\tSVM Class Label Generation Finished!");
		printf("\n");
	} else if (strcmp(msg, "PREDICT") == 0) {
		printf("\t\t\t\tAccuracy Testing Finished!");
		printf("\n");
	} else {
		printf("\t\t\t\tOff-line Training Data Collection Finished!");
		printf("\n");
	}

	close(sockfd);
}

int Sniffer::BcastSocket(const char* msgbuf, uint16_t port){
	struct sockaddr_in addr_server;
	int fd_server;
	
	if((fd_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		return 1;
	}
		memset(&addr_server, 0, sizeof(addr_server));
		addr_server.sin_family = AF_INET;
		addr_server.sin_addr.s_addr = inet_addr(GROUP);
		addr_server.sin_port = htons(port);

	if(sendto(fd_server,msgbuf,strlen(msgbuf),0,(struct sockaddr *)&addr_server,sizeof(addr_server))<0)
	{
		perror("send to");
		return 1;
	}
}

void Sniffer::NetSniffer() {

	/*---------------------Second Parameter, 70, Set the snapshot length to 70(34+?<70)---------------------------*/
	/*---------------------Third Parameter, 1, Turn promiscuous mode on-------------------------------------------*/
	/*---------------------Forth Parameter, 500, Set the timeout to 512 milliseconds------------------------------*/

	clock_t Sniffer_train_time_start;
	clock_t Sniffer_train_time_finish;

	// start clock time for Sniffer_train
	Sniffer_train_time_start = clock(); 

	Sniffer();
	GetTrainMAC();

	handle = pcap_open_live(SNIFFER_DEVICE, 70, 1, 500, errbuf);
	if (handle == NULL) {
		printf("pcap_create failed: %s\n", errbuf);
		exit(EXIT_FAILURE);
	}

	//printf("link-type: %d\n", pcap_datalink(handle));
	//printf("link-type: %s\n", pcap_datalink_val_to_name(pcap_datalink(handle)));

	u_char tx_mac_add[6];
	/*--------------------loop for 1 packet---------------------------*/
	int loop_num = 0;
	packCount = 0;
	while(onCollecting)
	{
		loop_num++;
		int loop = pcap_loop(handle,1, GetPacket, tx_mac_add);

		if(loop != 0)
		{
			printf("loop terminated before exhaustion: %d\n", loop);
		}
		
		if(packCount == 0 && loop_num > 1000)
		{
			memset(tx_mac_add,0,6);
			BcastSocket("FAIL",PORT1);
			onCollecting = 0;
			break;
		}

		if(packCount >= PACKETNUM || loop_num > SNIFFER_NUM)
		{
			memset(tx_mac_add,0,6);
			onCollecting = 0;
			break;
		}
	}

	pcap_close(handle);
	
	// finish clock time for Sniffer_train
	Sniffer_train_time_finish = clock(); 
	// print: time spent
	printf("\n Runtime : %f seconds.\n",
			((float) Sniffer_train_time_finish - (float) Sniffer_train_time_start) / CLOCK_PER_SEC);
}


int Sniffer::GetTrainID() {
	struct sockaddr_in addr;
	//int fd, nbytes,addrlen;
	int fd, nbytes;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];

	u_int yes = 1; /*** MODIFICATION TO ORIGINAL */

	while(1){
		if(onCollecting){
			
		}else{
			/* create what looks like an ordinary UDP socket */
			if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket");
				return 1;
			}

			/**** MODIFICATION TO ORIGINAL */
			/* allow multiple sockets to use the same PORT number */
			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
				perror("Reusing ADDR failed");
				return 1;
			}
			/*** END OF MODIFICATION TO ORIGINAL */

			/* set up destination address */
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY); /* N.B.: differs from sender */
			addr.sin_port = htons(PORT);

			/* bind to receive address */
			if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
				perror("bind");
				return 1;
			}

			/* use setsockopt() to request that the kernel join a multicast group */
			mreq.imr_multiaddr.s_addr = inet_addr(GROUP);
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			//mreq.imr_interface.s_addr=inet_addr("172.22.41.95");
			if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))
					< 0) {
				perror("setsockopt");
				return 1;
			}

			/* now just enter a read-print loop */
			//while (1) {
			socklen_t addrlen = sizeof(addr);
			memset(msgbuf, 0, MSGBUFSIZE);
			if ((nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0,
					(struct sockaddr *) &addr, &addrlen)) < 0) {
				perror("recvfrom");
				return 1;
			}
			/*------close client socket, start broad socket------*/
			close(fd);
			BcastSocket(msgbuf, PORT);
		
			onCollecting = 1;
			
			// generate local data from database
			if (strcmp(msgbuf, "LABEL") == 0) {
				//matrix generator
				if (AP_NO == 1) {
					char cmd_label[5];
					snprintf(cmd_label, 50, "./label.perl %d %d %d %s", AP_NUM, ROW, COLUMN, SERVERIP);
					
					// print but not execute
					//printf("%s\n",cmd_label);
					
					system(cmd_label);
					//system("./label.perl 5 4 12 192.168.1.110");
					
					ClientSocket(TRAINIP, msgbuf);
				}else{
					sleep(5);
				}
				onCollecting = 0;
			} 
			// generate svm model by training local data
			else if (strcmp(msgbuf, "MODEL") == 0) {
				//model generator
				if (AP_NO == 1) {
					char cmd_train[50];
					snprintf(cmd_train, 50, "./train.perl %d trainfile", AP_NUM);
					
					// print but not execute
					//printf("%s\n",cmd_train);
					
					system(cmd_train);
					//system("./train.perl 4 trainfile");
					
					ClientSocket(TRAINIP, msgbuf);
				}else{
					sleep(5);
				}
				onCollecting = 0;
			} 
			// do accuracy prediction for svm model
			else if (strcmp(msgbuf, "PREDICT") == 0) {
				//test accuracy
				if (AP_NO == 1) {
					char cmd_predict[50];
					snprintf(cmd_predict, 50, "./predict.perl %d %d", ROW, COLUMN );
					system(cmd_predict);
					ClientSocket(TRAINIP, msgbuf);
				}else{
					sleep(5);
				}
				onCollecting = 0;
			} 
			// data collection by sniffering network with specified MAC address
			else if (msgbuf[0]=='I' && msgbuf[1]=='D'){
				char message[256] = "\nID received:\t\tID = ";
				strcat(message, msgbuf+2);
				puts(message);
				memset(message, 0, MSGBUFSIZE);

				TrainID = atol(msgbuf+2);
				TrainBlock = TrainID / 10000;
				NetSniffer();

				memset(msgbuf, 0, sizeof(msgbuf));
				strcpy(msgbuf, "ID\n");
				char write_ap_no[3];
				int ap_no = AP_NO;
				snprintf(write_ap_no, 3, "%d", ap_no);
				strcat(msgbuf, write_ap_no);
				ClientSocket(TRAINIP, msgbuf);
			}
		}
	}
	return 0;
}

void* SnifferRun(void *argument) {
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
	AP_NO = atoi(no_in_string);
	//printf("AP_NO: %d\n",AP_NO);
	
	//start sniffering the network
	Sniffer snif;
	//parse arguements from input: trainIP, serverIP, row, column, number of APs, MAC of sniffered device
	snif.ParseArguments(argc, argv);
	//set monitor mode for wireless adapter
	snif.SetMode();
	//database connection
	snif.DatabaseConn();
	
	//heart of the program: the execution is based on command received from pad
	//1. sniffer packages
	//2. generate local data from database
	//3. train svm model
	//4. predict accracy of svm model
	snif.GetTrainID();
	
	//close database connection
	mysql_close(conn);
	mysql_library_end();
	
	return 0;
}
