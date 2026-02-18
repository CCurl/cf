CXX := clang
CFLAGS := -m32 -Oz

cf: *.c *.h
	$(CXX) $(CFLAGS) -o cf *.c
	ls -l cf

clean:
	rm -f cf

test: cf
	./cf boot.fth

run: cf
	./cf

bin: cf
	cp -u -p cf ~/bin/
