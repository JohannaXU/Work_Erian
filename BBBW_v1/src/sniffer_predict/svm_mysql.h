#ifndef SVM_MYSQL_H_
#define SVM_MYSQL_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mysql/mysql.h>

void DbConn(int argc, char **argv);
int svm_mysql(int argc, char **argv);
void* SvmPredict(void* arg);

#endif
