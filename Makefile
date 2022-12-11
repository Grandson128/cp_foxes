all:
	gcc -fopenmp ecosystem.c -o ecosystem
	gcc -fopenmp ecosystem_p.c -o ecosystem_p
