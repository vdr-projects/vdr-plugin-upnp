
parser: test_parser.cpp ../common/parser.cpp
	g++ -o test_parser test_parser.cpp ../common/parser.cpp -g -I../include -I/usr/include -I/usr/local/include
	
dllloader: dll_loader.cpp ../media/pluginManager.cpp ../common/tools.cpp
	g++ -o test_dllloader dll_loader.cpp ../media/pluginManager.cpp ../common/tools.cpp -g -ldl -Wl,--export-dynamic -I../include -I/usr/include -I/usr/local/include