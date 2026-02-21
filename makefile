ARCH ?= 64
CFLAGS = -m$(ARCH) -O3

cf: *.c *.h
	$(CC) $(CFLAGS) -o cf *.c
	ls -l cf

clean:
	rm -f cf

run: cf
	./cf

bin: cf cf-boot.fth
	cp -u -p cf ~/bin/
	cp -u -p cf-boot.fth ~/bin/
