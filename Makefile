all:
	gcc -c src/main.c -I./src -o src/main.o
	gcc -c src/base64.c -I./src -o src/base64.o
	gcc -c src/dex.c -I./src -o src/dex.o
	gcc -c src/leb128.c -I./src -o src/leb128.o
	gcc -c src/fill.c -I./src -o src/fill.o
	gcc src/*.o -o dex

debug:
	# cp ./test.dex.bak test.dex
	gcc -g -D_DEBUG_ -c src/main.c -I./src -o src/main.o
	gcc -g -D_DEBUG_ -c src/base64.c -I./src -o src/base64.o
	gcc -g -D_DEBUG_ -c src/dex.c -I./src -o src/dex.o
	gcc -g -D_DEBUG_ -c src/leb128.c -I./src -o src/leb128.o
	gcc -g -D_DEBUG_ -c src/fill.c -I./src -o src/fill.o
	gcc -g -D_DEBUG_ src/*.o -o dex-debug

test_b64:
	gcc -c test/b64_test.c -I./src -o test/b64_test.o
	gcc -c src/base64.c -I./src -o src/base64.o
	gcc test/b64_test.o src/base64.o -o test/b64_test

test_dex:
	gcc -g test/dex_test.c src/dex.c src/leb128.c src/base64.c -I./src/ -o test/dex

run:
	./dex-debug -off test.dex test.item

clean:
	rm -rf src/*.o
	rm -rf test/*.o test/b64_test