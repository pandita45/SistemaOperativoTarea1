shell: src/main.c src/miprof.c src/help.c
	gcc -o shell src/main.c
	gcc -o bin/miprof src/miprof.c
	gcc -o bin/help src/help.c
