#include "svm_mysql.h"
#include "svm.h"
#include "Sniffer.h"

struct svm_node *x;
struct svm_model* model;
MYSQL*	svm_conn;
int max_nr_attr = 64;
	
void DbConn(int argc, char **argv)
{
	// default values
	char* server_ip;
	server_ip = (char*) "192.168.1.189";

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (++i >= argc)
			exit(-1);
		switch (argv[i - 1][1]) {
		case 'n':	// number of APs
			break;
		case 's':	// server ip (database)
			//memset(server_ip, 0, 50); how to initialize?
			server_ip = argv[i];
			break;
		default:
			fprintf(stderr, "Unknown option: -%c\n", argv[i - 1][1]);
			exit(-1);
			break;
		}
	}
	
	if (mysql_library_init(0, NULL, NULL))
	{
	    fprintf(stderr, "could not initialize MySQL library\n");
	    exit(1);
	}
	
	svm_conn = mysql_init(NULL);

	//printf("if (!mysql_real_connect(svm_conn, %s, \"root\", \"root\", \"HAS\",0,NULL,0))\n", server_ip);

	if (!mysql_real_connect(svm_conn, server_ip, "root", "root", "HAS",0,NULL,0))
	//if (!mysql_real_connect(svm_conn, "localhost", "root", "root", "HAS",0,NULL,0))
	{
		printf( "Error connecting to database: %s\n",mysql_error(svm_conn));
	}
	
	while(1)
	{
		MYSQL_RES *store_result;
		MYSQL_ROW row, mysql_row;
		int query_result;
		unsigned int num_fields;
		unsigned int i;
		unsigned long *lengths;
		MYSQL_FIELD *fields;

		char mac_add[18];
		double predict_label;

		/*"select * from MACRecord where Position is NULL and
		 * ((RSSI1 and RSSI2 and RSSI3 and RSSI4 is not NULL)
		 * or timestampdiff(minute, Timestamp, now()) > 1)";*/

		//char query[512] = "select * from MACRecord where Position is NULL and ((";
		char query[512] = "select * from MACRecord where ((";
		//char query_second[255] = " is not NULL) or timestampdiff(second, Timestamp, now()) > 600)";
		char query_second[255] = " is not NULL) )";
		
		char query_col[512] =
				"select count(*) from information_schema.columns where table_name = \"MACRecord\" and column_name like \"RSSI%\"";
		query_result = mysql_real_query(svm_conn, query_col, (unsigned int) strlen(query_col));
		if (query_result) {
			printf("Error making query: %s\n", mysql_error(svm_conn));
		}

		store_result = mysql_store_result(svm_conn);
		mysql_row = mysql_fetch_row(store_result);
		int rssi_columns = atoi(mysql_row[0]);
		
		char write_and[] = " and ";
		char write_rssi_name[10];
		
		for(int rssi_no=1; rssi_no<=rssi_columns; rssi_no++)
		{
			snprintf(write_rssi_name, 10, "RSSI%d", rssi_no);
			strcat(query, write_rssi_name);
			if(rssi_no < rssi_columns)
			{
				strcat(query, write_and);
			}
		}
		strcat(query, query_second);
		/*----------end modification, need check--------*/
		
		char write_quot[]		= "\"";
		char write_where[]		= " where MACAdd = \"";
		char write_Position[5];
		
		query_result = mysql_real_query(svm_conn,query,(unsigned int) strlen(query));
		
		if (query_result)
		{
			printf("Error making query: %s\n",
			mysql_error(svm_conn));
		}

		store_result = mysql_store_result(svm_conn);

		num_fields = mysql_num_fields(store_result);
		fields = mysql_fetch_fields(store_result);
		
		while ((row = mysql_fetch_row(store_result)))
		{
			lengths = mysql_fetch_lengths(store_result);
			int rssi_index=0;

			int j = 0;
			for(i = 0; i < num_fields; i++)
			{
				if (strstr(fields[i].name, "MACAdd") != NULL )
				{
					strcpy(mac_add,row[i]);
					//strcpy(PredictMAC[num_rows],row[i]);
					//printf("%s\n",PredictMAC[num_rows]);
				}
				if (strstr(fields[i].name, "RSSI") != NULL )
				{
					rssi_index++;

					if ( (lengths[i]) > 1 )
					{
						x[j].index	= rssi_index;
						x[j].value	= atof(row[i]);
						j++;
					}
				 }
			}
			x[j].index = -1;
			//PredictLabel[num_rows]	= svm_predict(model,x);
			//printf("PredictLabel: %g\n", PredictLabel[num_rows]);

			predict_label = svm_predict(model,x);
			//printf("PredictLabel: %g\n", predict_label);

			char update_query[255]	= "update MACRecord set Timestamp = Timestamp, Position = ";
			snprintf(write_Position, 5, "%g", predict_label);
			strcat(update_query, write_Position);		// update MACRecord set Timestamp = Timestamp, Position = 1
			strcat(update_query, write_where);			// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "
			strcat(update_query, mac_add);				// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "CC:3A:61:CD:1B:96
			strcat(update_query, write_quot);			// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "CC:3A:61:CD:1B:96"
			//printf("%s\n",update_query);
			mysql_real_query(svm_conn,update_query,(unsigned int) strlen(update_query));
			//num_rows++;
		}
		mysql_free_result(store_result);
	}
}

/*
char PredictMAC[3000][18];
double PredictLabel[3000];
unsigned int num_rows;

void DbUpdatePos()
{
	char write_quot[]		= "\"";
	char write_where[]		= " where MACAdd = \"";
	char write_Position[5];
	char mac_add[18];
	for(int i = 0; i < num_rows; i++)
	{
		char update_query[255]	= "update MACRecord set Timestamp = Timestamp, Position = ";
		snprintf(write_Position, 5, "%g", PredictLabel[i]);
		strcat(update_query, write_Position);		// update MACRecord set Timestamp = Timestamp, Position = 1
		strcat(update_query, write_where);			// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "
		strcat(update_query, PredictMAC[i]);	// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "CC:3A:61:CD:1B:96
		strcat(update_query, write_quot);			// update MACRecord set Timestamp = Timestamp, Position = 1 where MACAdd = "CC:3A:61:CD:1B:96"
		printf("%s\n",update_query);
		mysql_real_query(svm_conn,update_query,(unsigned int) strlen(update_query));
	}
}
*/

int svm_mysql(int argc, char **argv)
{
	//num_rows	= 0;
	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	
	const char *model_file_name = "./model/trainfile.model"; 
	if((model=svm_load_model(model_file_name))==0)
	{
		fprintf(stderr,"can't open model file %s\n",model_file_name);
		exit(1);
	}
	
	DbConn(argc, argv);
	//DbUpdatePos();
	mysql_close(svm_conn);
	mysql_library_end();
	
	svm_free_and_destroy_model(&model);
	free(x);
	return 0;
}

void* SvmPredict(void* argument)
{
	//parse arguement from main.cpp
    arguments *arg	=(arguments *)argument;
    int argc	= arg->arg_char;
    char **argv	= arg->arg_vetcor;
	
	svm_mysql(argc, argv);
	return 0;
}
