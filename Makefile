brunnr: brunnr.c
	gcc -std=gnu11 -o brunnr -lsqlite3 brunnr.c
clean:
	rm brunnr
