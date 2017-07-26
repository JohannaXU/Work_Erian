//
//  main.cpp
//  sniffer
//
//  Created by yuehust on 13/3/14.
//  Copyright (c) 2014 yuehust. All rights reserved.
//

#include "Sniffer.h"
#include "svm_mysql.h"
#include "svm.h"

int main(int argc, char * argv[])
{
	pthread_t t5,t6;
	void *status;

	arguments m_arguments;

	m_arguments.arg_char	= argc;
	m_arguments.arg_vetcor	= argv;

	pthread_create( &t5, NULL, SnifferRun, (void *)&m_arguments);
	pthread_create( &t6, NULL, SvmPredict, (void *)&m_arguments);

	pthread_join(t5, &status);
	pthread_join(t6, &status);

	return 0;
}
