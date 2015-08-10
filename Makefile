linux:
	gcc -o mbpoll -L/usr/local/lib -I/usr/local/include mbpoll.c -lmodbus
	
windows:
	gcc -o mbpoll -L/usr/local/lib -I/usr/local/include mbpoll.c -lmodbus -lws2_32
