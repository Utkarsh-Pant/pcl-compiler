FLAGS := -Iinclude -g -Wpedantic -Wall
FILES := $(wildcard src/*.c src/helper/*.c)
INPUT := $(wildcard test/*.pcl)

test_file:
	gcc $(FLAGS) $(FILES) -o temp && \
	./temp $(file) && \
	rm -f ./temp

dev:
	gcc $(FLAGS) $(FILES) -o temp && \
	./temp $(INPUT) && \
	rm -f ./temp
	gcc -o p.out p.s 
bin:
	gcc $(FLAGS) $(FILES) -o temp
