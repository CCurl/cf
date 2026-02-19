ARCH ?= 64
CFLAGS = -m$(ARCH) -O3

cf: *.c *.h
	$(CC) $(CFLAGS) -o cf *.c
	ls -l cf

clean:
	rm -f cf

test: cf
	./cf boot.fth

run: cf
	./cf

bin: cf
	cp -u -p cf ~/bin/
