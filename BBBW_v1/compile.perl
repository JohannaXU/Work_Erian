#!/usr/bin/perl

$compile_sniffer = "g++ -o ./bin/Sniffer_train ./src/sniffer/main.cpp ./src/sniffer/Sniffer_train.cpp ./src/sniffer/Sniffer_train.h ./src/sniffer/ListenBroken.cpp ./src/sniffer/ListenBroken.h -lmysqlclient -lpcap -lpthread";
$compile_label = "g++ -o ./bin/label_generate ./src/label/label_generate.cpp -lmysqlclient";
$compile_train = "mpic++ -o ./bin/mpi-svm-train ./src/train/svm_for_train.cpp ./src/train/svm_for_train.h ./src/train/svm-train.c";
$compile_predict = "g++ -o ./bin/svm-predict ./src/predict/svm.cpp ./src/predict/svm.h ./src/predict/svm-predict.c";
$compile_sniffer_predict = "g++ -o ./bin/Sniffer_predict ./src/sniffer_predict/main.cpp ./src/sniffer_predict/Sniffer.cpp ./src/sniffer_predict/svm_mysql.cpp ./src/sniffer_predict/svm.cpp ./src/sniffer_predict/Sniffer.h ./src/sniffer_predict/svm_mysql.h ./src/sniffer_predict/svm.h -lmysqlclient -lpcap -lpthread";
$compile_accuracy_analysis = "g++ -o ./bin/accuracy-analysis ./src/accuracy-analysis/accuracy-analysis.cpp";


print "\t$compile_sniffer\n\n";
system("$compile_sniffer");
print "\t$compile_label\n\n";
system("$compile_label");
print "\t$compile_train\n\n";
system("$compile_train");
print "\t$compile_predict\n\n";
system("$compile_predict");
printf "\t$compile_sniffer_predict\n\n";
system("$compile_sniffer_predict");
printf "\t$compile_accuracy_analysis\n\n";
system("$compile_accuracy_analysis");