CC = gcc
TARGET = boilerplate

all:
	$(CC) boilerplate.c -o $(TARGET) -lbladeRF

clean:
	rm $(TARGET)


