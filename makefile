ARCH ?= 64
CFLAGS = -O3 -m$(ARCH)

cf: cf.c cf.h system.c
	$(CC) $(CFLAGS) cf.c system.c -o $@

run: cf
	./cf

clean:
	rm -f cf

test: cf
	./cf block-200.cf

bin: cf
	cp -u -p cf ~/bin/
