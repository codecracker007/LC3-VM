all: vm

vm: main.o vm.o
	gcc -Wall -o vm main.o vm.o

vm.o: vm.c
	gcc -c -Wall vm.c

main.o: main.c
	gcc -c -Wall main.c

clean:
	rm main.o
	rm vm.o

