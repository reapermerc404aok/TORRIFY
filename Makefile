all:
	gcc torrify.c -o torrify.so -fPIC -shared -ldl -D_GNU_SOURCE
