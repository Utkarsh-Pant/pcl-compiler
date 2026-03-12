FLAGS := -Iinclude -g
FILES := $(wildcard src/*.c src/helper/*.c)
INPUT := $(wildcard test/*.pcl)

test_file:
	gcc $(FLAGS) $(FILES) -o temp && \
	./temp $(file) && \
	rm -rf ./temp

dev:
	gcc $(FLAGS) $(FILES) -o temp && \
	./temp $(INPUT) && \
	rm -rf ./temp

bin:
	gcc $(FLAGS) $(FILES) -o temp
