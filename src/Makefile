all: client server

client:
	g++ -o sender.out client.cpp prompts.h fileIO.h packetIO.h sitErrors.h

server:
	g++ -o reciever.out server.cpp prompts.h fileIO.h packetIO.h
	
clean:server client
	rm server
	rm client