CC = gcc
TARGET1 = boilerplate
TARGET2 = rx
boilerplate: boilerplate.c
	$(CC) boilerplate.c -o $(TARGET1) -lbladeRF

rx: rx.c
	$(CC) rx.c -o rx -lbladeRF

clean:
	rm $(TARGET1) $(TARGET2)


