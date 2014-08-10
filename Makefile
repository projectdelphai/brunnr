brunnr: brunnr.c
	gcc -std=gnu11 -o brunnr brunnr.c -lsqlite3
clean:
	rm brunnr
