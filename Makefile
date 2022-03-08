all: ringmaster player
ringmaster: potato.o ringmaster.o server.o
	g++ -o ringmaster ringmaster.o server.o
player: potato.o player.o server.o
	g++ -o player player.o server.o
ringmaster.o: ringmaster.cpp
	g++ $(CPPFLAGS) -c ringmaster.cpp
player.o: player.cpp
	g++ $(CPPFLAGS) -c player.cpp
server.o: server.hpp server.cpp
	g++ $(CPPFLAGS) -c server.cpp

potato.o: potato.hpp potato.cpp
	g++ $(CPPFLAGS) -c potato.cpp

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player
