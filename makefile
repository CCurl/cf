app := cf
app32 := cf32

CXX := clang
CXXFLAGS := -O3 -D IS_LINUX

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) -m64 -D _M64_ $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

$(app32): $(srcfiles) $(incfiles)
	$(CXX) -m32 $(CXXFLAGS) $(LDFLAGS) -o $(app32) $(srcfiles) $(LDLIBS)
	ls -l $(app32)

clean:
	rm -f $(app) $(app32)

run: $(app)
	./$(app)

test: $(app)
	./$(app) block-200.cf

bin: $(app)
	cp -u -p $(app) ~/bin/
