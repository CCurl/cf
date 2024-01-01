app32 := cf32
app64 := cf64
app := $(app32)

CXX := clang
CXXFLAGS := -O3 -D IS_LINUX

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app64) $(app32)

$(app64): $(srcfiles) $(incfiles)
	$(CXX) -m64 -D _M64_ $(CXXFLAGS) $(LDFLAGS) -o $(app64) $(srcfiles) $(LDLIBS)
	ls -l $(app64)

$(app32): $(srcfiles) $(incfiles)
	$(CXX) -m32 $(CXXFLAGS) $(LDFLAGS) -o $(app32) $(srcfiles) $(LDLIBS)
	ls -l $(app32)

clean:
	rm -f $(app64) $(app32)

test: $(app)
	./$(app) test1.txt

bin: $(app)
	cp -u -p $(app) ~/.local/bin/
