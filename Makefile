
TARGET ?= target/psse
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INDENT_FLAGS := -kr -br -brf -brs -ce -cdw -npsl -nut -prs -l79 -ts2

VERSION := "0.0.0"

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g
LDFLAGS := -lm

$(TARGET): $(OBJS) Makefile
	$(CC) $(LDFLAGS) $(OBJS) -DVERSION=$(VERSION) -o $@ $(LDFLAGS) $(LOADLIBES) $(LDLIBS)

format:
	indent $(INDENT_FLAGS) $(SRCS) src/*.h

test:
	bash ./unit-tests.sh

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

repl:
	$(TARGET) -p 2> psse.log


-include $(DEPS)
