#include "Sniffer_train.h"
#include "ListenBroken.h"

int ListenBroken() {
	struct sockaddr_in addr, addr_server;
	//int fd, nbytes,addrlen;
	int fd, fd_server, nbytes;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];

	u_int yes = 1; /*** MODIFICATION TO ORIGINAL */

	while(1){
		if(onCollecting){
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
			addr.sin_port = htons(PORT1);

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
			
			if (strcmp(msgbuf, "FAIL") == 0) {
				onCollecting = 0;
				//BcastSocket(msgbuf, PORT1);
				if((fd_server = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
				{
					perror("socket");
					return 1;
				}
				memset(&addr_server, 0, sizeof(addr_server));
				addr_server.sin_family = AF_INET;
				addr_server.sin_addr.s_addr = inet_addr(GROUP);
				addr_server.sin_port = htons(PORT1);

				if(sendto(fd_server,msgbuf,strlen(msgbuf),0,(struct sockaddr *)&addr_server,sizeof(addr_server))<0)
				{
					perror("send to");
					return 1;
				}
			} 
		}
	}
	return 0;
}

void *CommPadBoard(void *argument){
	ListenBroken();
	return 0;
}
