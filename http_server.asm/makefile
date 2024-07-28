build:
	nasm -Isrc/ -f elf64 -g src/server.asm
	gcc -no-pie src/server.o -o src/server

run:
	./src/server

clean:
	rm -rf src/*.o src/server
