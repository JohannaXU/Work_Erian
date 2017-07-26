2014/12/02
Update:
1. act as a client in broadcast procedure
2. capture more than 1 packets

Sniffer for training data collection

command:
g++ -o Sniffer_train main.cpp Sniffer_train.cpp Sniffer_train.h -lmysqlclient -lpcap -lpthread

progress:
1. describe MACTrain;  
+-----------+------------+------+-----+-------------------+-----------------------------+
| Field     | Type       | Null | Key | Default           | Extra                       |
+-----------+------------+------+-----+-------------------+-----------------------------+
| ID        | int(11)    | YES  | UNI | NULL              |                             |
| Timestamp | timestamp  | NO   |     | CURRENT_TIMESTAMP | on update CURRENT_TIMESTAMP |
| RSSI1     | tinyint(4) | YES  |     | NULL              |                             |
| RSSI2     | tinyint(4) | YES  |     | NULL              |                             |
+-----------+------------+------+-----+-------------------+-----------------------------+

2. In the new table, MACAdd is unique, and two RSSI included(RSSI1, RSSI2).

3. If new RSSI to be included. 
   for Sniffer.h, change AP_NUM to 3
   for mysql
   > alter table MACRecord add RSSI3 TINYINT;

4. Different RSSI, different value for AP_NO in Sniffer.h

6. SNIFFER_DEVICE "wlan0" need to be modified for different gates
