all: tlsproxy

tlsproxy: *.c tls_help/*.c
	$(CC) -o $@ -Iinclude -I. $^

clean:
	rm -f tlsproxy
