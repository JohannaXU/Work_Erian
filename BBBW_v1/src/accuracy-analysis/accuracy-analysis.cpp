/*
 * Input file: first column (real position) other columns (predicted positions, left -> right: possibility high -> low)
 * Input file: file name: out
 * Result:
 * For BLOCK 48, it will generate a 48 x 48 matrix
 * The result's (matrix[real][predict]) meaning:
 * x index: actual position
 * y index: predicted position
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
using namespace std;

//#define BLOCK 48
//#define DISTANCE 6
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

char *line = NULL;
int max_line_len = 1024;
int distance = 0;
int row = 4;
int col = 12;

void parse_command_line(int argc, char **argv, char *input_file_name)
{
	int i;
	
	distance = 0;

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		if(++i>=argc){
			fprintf(stderr,"error\n");
			exit(-1);
		}
		switch(argv[i-1][1])
		{
			case 'd':
				distance = atoi(argv[i]);
				break;
			case 'r':
				row = atoi(argv[i]);
				break;
			case 'c':
				col = atoi(argv[i]);
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				exit(-1);
		}
	}
	
	strcpy(input_file_name, argv[i]);

}

	
char* readline(FILE *input)
{
	int len;
	
	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

bool isNear(int i, int j, int distance)
{
	int ix, iy, jx, jy;
	
	ix = (i-1)%col;
	iy = (i-1)/col;
	
	jx = (j-1)%col;
	jy = (j-1)/col;
	
	if(abs(ix-jx)<=distance && abs(iy-jy)<=distance)
	{
		return true;
	}
	
	
/*	if (abs(i - j) == 1) {
		if ((i < j and j % col == 0) or (i > j and i % col == 0)) {
			return false;
		} else {
			return true;
		}

	} else if ((abs(i - j) == col)) {
		return true;
	}
*/	
	return false;
}

int main(int argc, char **argv)
{
	char input_file_name[1024];
	
	parse_command_line(argc, argv, input_file_name);
	
	FILE *fp = fopen(input_file_name,"r");
	
	int block_num = row*col;

	int matrix[block_num][block_num];
	
	long total_instances = 0;
	long correct_instances = 0;
	
	for(int i=0;i<block_num;i++){
		for(int j=0;j<block_num;j++){
			matrix[i][j] = 0;
		}
	}
	int block_real, block_predict;
	//char* left_num, right_num;
	
	line = Malloc(char,max_line_len);
	
	while(readline(fp)!=NULL)
	{
		char *left_num = strtok(line,"\t");
		block_real = (int) strtol(left_num,NULL,10);
		//printf("%d\t",block_real);
		
		char *right_num = strtok(NULL,"\t\n");
		block_predict = (int) strtol(right_num,NULL,10);
		//printf("%d\t\n",block_predict);
		
		int real, predict;
		real = block_real-1;
		predict = block_predict-1;
		matrix[real][predict]++;
	}
	
	for(int i=0;i<block_num;i++){
		for(int j=0;j<block_num;j++){
			total_instances += matrix[i][j];
			if(isNear(i,j,distance)){
				correct_instances += matrix[i][j];
			}
			//printf("%d\t",matrix[i][j]);
		}
		//printf("\n");
	}
	
	printf("\t\t%d\t%.2f%%\n", distance, ((float)correct_instances)/((float)total_instances)*100);
	free(line);
	return 0;
}
