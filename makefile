OPT = -Wall -pedantic

main: ./src/main.c
	gcc ${OPT} -o main ./src/main.c

clean: 
	rm main