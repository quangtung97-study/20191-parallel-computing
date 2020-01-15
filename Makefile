.PHONY: count

count:
	fd | grep main.c | xargs wc -l
