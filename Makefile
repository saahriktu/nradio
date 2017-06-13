all:
	gcc -o nradio nradio.c -lcurses -lmenu
install:
	cp nradio /usr/bin
