CC := gcc

WARNING_FLAGS := -Wall -Wextra -pedantic
DEBUG_FLAGS := -g -fsanitize=address

FEED_OUTPUT_NAME := feed
MANAGER_OUTPUT_NAME := manager

######
# General rules
######

default: all

all: feed manager

feed: feed.o 
	$(CC) feed.o -o $(FEED_OUTPUT_NAME) $(WARNING_FLAGS)

manager: manager.o
	$(CC) manager.o -o $(MANAGER_OUTPUT_NAME) $(WARNING_FLAGS)

feed.o: feed.c util.h
	$(CC) feed.c -c $(WARNING_FLAGS)

manager.o: manager.c util.h
	$(CC) manager.c -c $(WARNING_FLAGS)

######
# Debug rules
######

all-debug: feed-debug manager-debug

feed-debug: feed.c util.h
	$(CC) feed.c -o feed $(WARNING_FLAGS) $(DEBUG_FLAGS)

manager-debug: manager.c util.h
	$(CC) manager.c -o manager $(WARNING_FLAGS) $(DEBUG_FLAGS)

######
# Clean rules
######

clean: clean-obj clean-exe

clean-obj:
	rm *.o -f

clean-exe: 
	rm -f $(FEED_OUTPUT_NAME)
	rm -f $(MANAGER_OUTPUT_NAME)

clean-pipes:
	rm -f f_*
	rm -f tubo
