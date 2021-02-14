cd ./client
gcc ./client.c -lws2_32 -lShlwapi -I../core -o ./client && ./client.exe