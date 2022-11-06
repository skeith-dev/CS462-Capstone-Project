all: client server

client:
	g++ -o sender.out client.cpp packet.h
	
server:
	g++ -o reciever.out server.cpp packet.h
	
clean:server client
	rm server
	rm client