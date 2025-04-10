all: broker subscriber

broker:
	gcc broker.c -o broker -pthread

subscriber:
	gcc subscriber.c -o subscriber

.PHONY: clean
clean:
	rm -f broker
	rm -f subscriber

