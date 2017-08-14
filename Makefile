
TARGET ?= target/psse
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
INDENT_FLAGS := -kr -nut -l79 -ts2

VERSION := "0.0.0"

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
LDFLAGS := -lm

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -DVERSION=$(VERSION) -o $@ $(LDFLAGS) $(LOADLIBES) $(LDLIBS)

format:
	indent $(INDENT_FLAGS) $(SRCS) src/*.h

test:
	bash ./unit-tests.sh

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
