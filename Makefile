
TARGET ?= target/psse
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
HDRS := $(shell find $(SRC_DIRS) -name *.h)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

TESTS := $(shell find unit-tests -name *.sh)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INDENT_FLAGS := -kr -br -brf -brs -ce -cdw -npsl -nut -prs -l79 -ts2

VERSION := "0.0.0"

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g
LDFLAGS := -lm

$(TARGET): $(OBJS) Makefile
	$(CC) $(LDFLAGS) $(OBJS) -DVERSION=$(VERSION) -o $@ $(LDFLAGS) $(LOADLIBES) $(LDLIBS)

doc: $(SRCS) Makefile
	doxygen

format: $(SRCS) $(HDRS) Makefile
	indent $(INDENT_FLAGS) $(SRCS) $(HDRS)

test: $(OBJS) $(TESTS) Makefile
	bash ./unit-tests.sh

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS) $(SRC_DIRS)/*~

repl:
	$(TARGET) -p 2> psse.log


-include $(DEPS)
