
# sudo apt-get install libsqlite3-dev

server:server.c
	gcc $^ -o $@ -lsqlite3 -lpthread

.PHONY:clean
clean:
	rm server
