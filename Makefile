TARGET ?= target/psse
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
HDRS := $(shell find $(SRC_DIRS) -name *.h)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

TESTS := $(shell find unit-tests -name *.sh)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

INDENT_FLAGS := -nbad -bap -nbc -br -brf -brs -c33 -cd33 -ncdb -ce -ci4 -cli4 \
-d0 -di1 -nfc1 -i4 -ip0 -l75 -lp -npcs \
-npsl -nsc -nsob -nss -nut -prs -l79 -ts2

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -DDEBUG
LDFLAGS := -lm -lcurl
DEBUGFLAGS := -g3

all: $(TARGET)

$(TARGET): $(OBJS) Makefile
	$(CC) $(DEBUGFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LOADLIBES) $(LDLIBS)

doc: $(SRCS) Makefile Doxyfile
	doxygen

format: $(SRCS) $(HDRS) Makefile
ifeq ($(shell uname -s), Darwin)
	gindent $(INDENT_FLAGS) $(SRCS) $(HDRS)
else
	indent $(INDENT_FLAGS) $(SRCS) $(HDRS)
endif

test: $(OBJS) $(TESTS) Makefile
	bash ./unit-tests.sh

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(SRC_DIRS)/*~ $(SRC_DIRS)/*/*~ *~ core

repl:
	$(TARGET) -p 2> psse.log


-include $(DEPS)
