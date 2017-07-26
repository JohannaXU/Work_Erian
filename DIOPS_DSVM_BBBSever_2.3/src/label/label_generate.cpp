/*-------------------------Compile-------------------------*/
// g++ -o label_generate label_generate.cpp -lmysqlclient
/*-------------------------Run-----------------------------*/
// ./label_generate -r 4 -c 12 -h 172.21.154.123 -t 50
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>       /* time */
#include <iostream>
#include <sstream>
#include <cmath>
#include <mysql/mysql.h>
#include <sstream>
#include <algorithm>
#include <unistd.h>

void exit_with_help();
void parse_arguments(int argc, char **argument);
float** generate_avg_rssi_table(const int row, const int col);
float** generate_block_dist(const int row, const int col, float** avg_rssi_table);
int** generate_distance_matrix(int row, int col);
int** generate_class_matrix(int row, int col, float threshold);
void find_nearby_blocks(int size, int i, int* labels, int max_block,
		int** distance_matrix, int** class_matrix);
void mysql_update_label(int i, int* label);
void apply_class_labels(int row, int col, int max_block, int** distance_matrix,
		int** class_matrix);
void generate_csv_file(int generate_csv_file);
int svm_generate_data(const char *datafile, const char *trainfile,
		const char *testfile);

using namespace std;

MYSQL* conn;

struct label_generate_param {
	int row;
	int column;
	int max_block;
	char* ip;
	float threshold;
	char* datafile;
	char* trainfile;
	char* testfile;
} param;

void exit_with_help() {
	printf(
			"Usage: matrix -r [number_rows] -c [number_columns] -b [blocks] -h [MySQL_Server_IP] -t [threshold] -fd [datafile] -ft [trainfile] -fp [testfile]\n"
					"options:\n"
					"-r rows : set row number of blocks in testing area\n"
					"-c columns : set column number of blocks in testing ara\n"
					"-b blocks: set the maximum number of blocks a class can have\n"
					"-h MySQL IP : set IP of MySQL database server\n"
					"-t threshold : set threshold of separating blocks, higher the threshold, less blocks will be generated, default is 50\n"
					"-fd datafile: set datafile to store all records\n"
					"-ft trainfile: set trainfile to store train records\n"
					"-fp testfile/predctfile set testfile to store test records\n");
	exit(1);
}

void parse_arguments(int argc, char **argv) {

	// default values
	param.row = 4;
	param.column = 12;
	param.max_block = 5;
	param.threshold = 5;
	param.ip = (char*) "localhost";
	param.datafile = (char*) "datafile";
	param.trainfile = (char*) "trainfile";
	param.testfile = (char*) "testfile";

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			break;
		if (++i >= argc)
			exit_with_help();
		switch (argv[i - 1][1]) {
		case 'r':
			param.row = atoi(argv[i]);
			break;
		case 'c':
			param.column = atoi(argv[i]);
			break;
		case 'b':
			param.max_block = atoi(argv[i]);
			break;
		case 'h':
			param.ip = argv[i];
			break;
		case 't':
			param.threshold = atof(argv[i]);
			break;
		case 'f':
			switch (argv[i - 1][2]) {
			case 'd':
				param.datafile = argv[i];
				break;
			case 't':
				param.trainfile = argv[i];
				break;
			case 'p':
				param.testfile = argv[i];
				break;
			default:
				fprintf(stderr, "Unknown option: -%c%c\n", argv[i - 1][1],
						argv[i - 1][2]);
				exit_with_help();
				break;
			}
			break;
		default:
			fprintf(stderr, "Unknown option: -%c\n", argv[i - 1][1]);
			exit_with_help();
			break;
		}
	}

	if (mysql_library_init(0, NULL, NULL)) {
		fprintf(stderr, "could not initialize MySQL library\n");
		exit(1);
	}

	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, param.ip, "root", "root", "HAS", 0, NULL,
			0)) {
		printf("Error connecting to database: %s\n", mysql_error(conn));
	}

}

// generate cluster analysis matrix --> generate average rssi table
float** generate_avg_rssi_table(const int row, const int col){

	int block_number = row * col;
	float** avg_rssi_table;

	MYSQL_RES *store_result;
	MYSQL_ROW mysql_row;

	int query_result;
	char query[512] =
			"select count(*) from information_schema.columns where table_name = \"MACTrain\" and column_name like \"RSSI%\"";
	query_result = mysql_real_query(conn, query, (unsigned int) strlen(query));

	if (query_result) {
		printf("Error making query: %s\n", mysql_error(conn));
	}

	store_result = mysql_store_result(conn);
	mysql_row = mysql_fetch_row(store_result);

	int rssi_columns = atoi(mysql_row[0]);

	avg_rssi_table = new float*[block_number];
	for (int i = 0; i < block_number; i++) {
		int j;
		memset(query, 0, sizeof(query));
		avg_rssi_table[i] = new float[rssi_columns];
		for (j = 0; j < rssi_columns; j++) {
			avg_rssi_table[i][j] = 0;
			snprintf(query, 512,
					"SELECT AVG(RSSI%d) FROM MACTrain where Block = %d",
					j + 1, i + 1);
			query_result = mysql_real_query(conn, query,
					(unsigned int) strlen(query));
			if (query_result) {
				printf("Error making query: %s\n", mysql_error(conn));
			}

			store_result = mysql_store_result(conn);
			mysql_row = mysql_fetch_row(store_result);

			avg_rssi_table[i][j] = atof(mysql_row[0]);
			//printf("%.2f\t",avg_rssi_table[i][j]);
		}
		//printf("\n");
	}

	mysql_free_result(store_result);
	return avg_rssi_table;
}

// generate block distance matrix
float** generate_block_dist(const int row, const int col, float** avg_rssi_table) {
	float** block_dist;
	int block_number = row * col;

	MYSQL_RES *store_result;
	MYSQL_ROW mysql_row;

	int query_result;
	char query[512] =
			"select count(*) from information_schema.columns where table_name = \"MACTrain\" and column_name like \"RSSI%\"";
	query_result = mysql_real_query(conn, query, (unsigned int) strlen(query));

	if (query_result) {
		printf("Error making query: %s\n", mysql_error(conn));
	}

	store_result = mysql_store_result(conn);
	mysql_row = mysql_fetch_row(store_result);

	int rssi_columns = atoi(mysql_row[0]);

	block_dist = new float*[block_number];
	for (int i = 0; i < block_number; i++) {
		block_dist[i] = new float[block_number];
		for (int j = 0; j < block_number; j++) {
			block_dist[i][j] = 0.0;
			for (int k = 0; k < rssi_columns; k++) {
				block_dist[i][j] += abs(avg_rssi_table[i][k] - avg_rssi_table[j][k]);
			}
			//printf("%.2f\t",block_dist[i][j]);
			//cout << block_dist[i][j] << "\t";
		}
		//cout << "\n";
	}
	//cout << "\n";
	return block_dist;
}

// generate a matrix to describe the related position of different blocks
// if block i and block j are nearby, then distance_matrix[i][j] = 1, else 0
int** generate_distance_matrix(int row, int col) {
	int** distance_matrix = 0;
	int matrix_size = row * col;
	distance_matrix = new int*[matrix_size];
	for (int i = 0; i < matrix_size; i++) {
		distance_matrix[i] = new int[matrix_size];
		for (int j = 0; j < matrix_size; j++) {
			if (abs(i - j) == 1) {
				if ((i < j and j % col == 0) or (i > j and i % col == 0)) {
					distance_matrix[i][j] = 0;
				} else {
					distance_matrix[i][j] = 1;
				}

			} else if ((abs(i - j) == col)) {
				distance_matrix[i][j] = 1;
			} else {
				distance_matrix[i][j] = 0;
			}
			//cout << distance_matrix[i][j] << "\t";
		}
		//cout << "\n";
	}
	//cout << "\n";
	return distance_matrix;
}

// generate initial class matrix to describe whether different blocks have the same rssi profile
int** generate_class_matrix(int row, int col, float threshold) {
	int size = row * col;
	int ** class_matrix;

	float** matrix = generate_block_dist(row, col, generate_avg_rssi_table(row, col));
	class_matrix = new int*[size];
	for (int i = 0; i < size; i++) {
		class_matrix[i] = new int[size];
		for (int j = 0; j < size; j++) {
			class_matrix[i][j] = 0;
			//cout << matrix[i][j] << "\t";
			if (matrix[i][j] < threshold) {
				class_matrix[i][j] = 1;
			} else {
				class_matrix[i][j] = 0;
			}
			//cout << class_matrix[i][j] << "\t";
		}
		//cout << "\n";
	}
	//cout << "\n";
	return class_matrix;
}

// find nearby blocks with the same class and apply with lables

void find_nearby_blocks(int size, int start_block, int i, int* labels, int max_block,
		int** distance_matrix, int** class_matrix) {
				
	for (int j = 0; j < size; j++) {
		if (labels[j] == 0 && distance_matrix[i][j] == 1 && class_matrix[start_block][j] == 1) {
			int count = 0;				
			for (int k = 0; k < size; k++) {
				if (labels[k] == labels[start_block]) {
					count++;
					if (count >= max_block) {
						count = 0;
						return;
					}
				}
			}
			labels[j] = labels[start_block];
			find_nearby_blocks(size, start_block, j, labels, max_block, distance_matrix,
						class_matrix);
		}
	}
}

/*
void find_nearby_blocks(int size, int i, int* labels, int max_block,
		int** distance_matrix, int** class_matrix) {

	int count = 0;

	for (int j = 0; j < size; j++) {
		if (labels[j] == 0 && distance_matrix[i][j] == 1) {
			for (int k = 0; k < size; k++) {
				if (labels[k] == labels[i]) {
					count++;
					if (count >= max_block || class_matrix[k][j] == 0) {
						count = 0;
						return;
					}
				}
			}
			labels[j] = labels[i];
			find_nearby_blocks(size, j, labels, max_block, distance_matrix,
					class_matrix);
		}
	}
}
*/
// update label on MACTrain table
void mysql_update_label(int i, int* label) {

	char write_label[4];
	char write_block[5];

	char query[255] = "UPDATE MACTrain SET LABEL=";
	char query2[255] = "  WHERE Block= ";

	snprintf(write_label, 4, "%i", label[i]);
	snprintf(write_block, 6, "%i", i + 1);
	// UPDATE MACTrain SET LABEL=15
	strcat(query, write_label);
	// UPDATE MACTrain SET LABEL=15  WHERE Block=
	strcat(query, query2);
	// UPDATE MACTrain SET LABEL=15  WHERE Block= 48
	strcat(query, write_block);

	mysql_real_query(conn, query, (unsigned int) strlen(query));
}

void apply_class_labels(int row, int col, int max_block, int** distance_matrix,
		int** class_matrix) {

	int size = row * col;

	int labels[size];
	for (int i = 0; i < size; i++) {
		labels[i] = 0;
	}

	//Search for nearby nodes and update class labels
	for (int i = 0; i < size; i++) {
		if (labels[i] == 0) {
			// Search for the maximum label existed, and set new label 1 larger.
			labels[i] = *std::max_element(labels, labels + size) + 1;
			// Find the nearby blocks for new label
			find_nearby_blocks(size, i, i, labels, max_block, distance_matrix,
					class_matrix);
		}
	}

	// Print the results of labeled blocks
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			cout << labels[i * col + j] << "\t";
		}
		cout << "\n";
	}

	// Update all the labels on MACTrain table
	for (int i = 0; i < size; i++) {
		mysql_update_label(i, labels);
	}
}

// Generate .csv file for weka testing
// remove data files into separate files
void generate_csv_file(float threshold) {

	char current_working_directory[1024];
	getcwd(current_working_directory, 1024);

	char folder_rm[255];
	char folder_create[255];

	char csv_generate[255];
	char csv_mv[255];

	char datafile_cp[255];

	snprintf(csv_generate, 255,
			"SELECT 'Block','ID','RSSI1','RSSI2','RSSI3','RSSI4','LABEL' UNION ALL "
					"SELECT Block, ID, RSSI1,RSSI2,RSSI3,RSSI4,LABEL FROM MACTrain into "
					"OUTFILE '/tmp/weka_file_%.2f.csv' "
					"FIELDS ENCLOSED BY '\"'  ESCAPED BY '\"' LINES TERMINATED BY '\\r\\n'",
			threshold);
	mysql_real_query(conn, csv_generate, strlen(csv_generate));

	snprintf(folder_rm, 255, "sudo rm -rf folder_%.2f", threshold);
	snprintf(folder_create, 255, "sudo mkdir folder_%.2f", threshold);

	snprintf(csv_mv, 255, "sudo mv /tmp/weka_file_%.2f.csv %s/folder_%.2f",
			threshold, current_working_directory, threshold);

	snprintf(datafile_cp, 255,
			"sudo cp ./data/%s ./data/%s ./data/%s folder_%.2f", param.datafile,
			param.trainfile, param.testfile, threshold);

//	cout << folder_rm << "\n";
//	cout << folder_create << "\n";
//	cout << csv_mv << "\n";
//	cout << datafile_cp << "\n";

	system(folder_rm);
	system(folder_create);
	system(csv_mv);
	//system(datafile_cp);
}

// generate datafile(100%); trainfile(90%); testfile(10%)
int svm_generate_data(const char *datafile, const char *trainfile,
		const char *testfile) {
	FILE *fp_data = fopen(datafile, "w");
	FILE *fp_train = fopen(trainfile, "w");
	FILE *fp_test = fopen(testfile, "w");
	if (fp_data == NULL)
		return -1;
	if (fp_train == NULL)
		return -1;
	if (fp_test == NULL)
		return -1;

	MYSQL_RES *store_result;
	MYSQL_ROW row;

	int query_result;

	const char* query = "select * from MACTrain";
	query_result = mysql_real_query(conn, query, (unsigned int) strlen(query));

	if (query_result) {
		printf("Error making query: %s\n", mysql_error(conn));
	}

	store_result = mysql_store_result(conn);

	int label;

	unsigned int num_fields;
	unsigned int i;
	unsigned long *lengths;
	MYSQL_FIELD *fields;

	num_fields = mysql_num_fields(store_result);
	fields = mysql_fetch_fields(store_result);

	label = 0;
	int percent = 10; // percentage for test is 1/percent
	while ((row = mysql_fetch_row(store_result))) {

		lengths = mysql_fetch_lengths(store_result);
		for (i = 0; i < num_fields; i++) {
			if (strstr(fields[i].name, "LABEL") != NULL) {
				if (label == percent) {
					label = 0;
				}
				fprintf(fp_data, "%s\t", row[i]);
				if (label < percent-1) {
					fprintf(fp_train, "%s\t", row[i]);
				} else {
					fprintf(fp_test, "%s\t", row[i]);
				}
				int rssi_index = 0;

				for (i = 0; i < num_fields; i++) {
					if (strstr(fields[i].name, "RSSI") != NULL) {
						rssi_index++;
						if ((lengths[i]) > 1) {
							fprintf(fp_data, "%i:%s\t", rssi_index, (row[i]));
							if (label < percent-1) {
								fprintf(fp_train, "%i:%s\t", rssi_index,
										(row[i]));
							} else {
								fprintf(fp_test, "%i:%s\t", rssi_index,
										(row[i]));
							}
						}
					}
				}
				fprintf(fp_data, "\n");
				if (label < percent-1) {
					fprintf(fp_train, "\n");
				} else {
					fprintf(fp_test, "\n");
				}
			}
		}
		label++;
	}
	// store records in data folder
	char move_file[255];
	snprintf(move_file, 255, "mv %s %s %s ./data", datafile, trainfile,
			testfile);
	system(move_file);

	mysql_free_result(store_result);
	return 0;
}

int main(int argc, char *argv[]) {
	parse_arguments(argc, argv);

	int row = param.row;
	int col = param.column;
	int max_block = param.max_block;
	float threshold = param.threshold;

	const char* filename_data = param.datafile;
	const char* filename_train = param.trainfile;
	const char* filename_test = param.testfile;

	int** distance_matrix = generate_distance_matrix(row, col);
	int** class_matrix = generate_class_matrix(row, col, threshold);

	apply_class_labels(row, col, max_block, distance_matrix, class_matrix);

	svm_generate_data(filename_data, filename_train, filename_test);

	//generate_csv_file(threshold);

	return 0;
}
