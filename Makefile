all:
	echo "break"

test_b64:
	gcc -c test/b64_test.c -I./src -o test/b64_test.o
	gcc -c src/base64.c -I./src -o src/base64.o
	gcc test/b64_test.o src/base64.o -o test/b64_test