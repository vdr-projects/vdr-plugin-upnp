parser: test_parser.cpp ../common/parser.cpp
	g++ -o test_parser test_parser.cpp ../common/parser.cpp -g -I../include -I/usr/include -I/usr/local/include

dllloader: dll_loader.cpp
	g++ -o test_dllloader dll_loader.cpp  -g -I../include -I/usr/include -I/usr/local/include

database: test_database.cpp
	g++ -o test_database test_database.cpp -g -lsqlite3 -I../include -I/usr/include -I/usr/local/include