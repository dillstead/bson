NDEBUG ?= 0
NASAN ?= 0

CC        := gcc
NAME      := bson
LIB       := lib$(NAME).so
TST       := $(NAME)_test
BUILDDIR  := build
SRC       := str.c toker.c arena.c bson_int.c bson_str.c bson_list.c \
	bson_obj.c bson.c 
OBJ       := $(SRC:%.c=$(BUILDDIR)/%.o)
TST_SRC   := test.c test_utils.c test_tok.c test_types.c test_parse.c \
	test_arena.c
TST_OBJ   := $(TST_SRC:%.c=$(BUILDDIR)/%.o)
CFLAGS    := -std=c11 -Wall -Wextra -I./include
LDFLAGS   := -L.
ifeq ($(NDEBUG), 0)
CFLAGS    += -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fsanitize=undefined -g3
LDFLAGS   += -fsanitize=undefined 
ifeq ($(NASAN), 0)
CFLAGS += -fsanitize=address
LDLIBS := -lasan
endif
else
CFLAGS    += -O2 -DNDEBUG
endif

.PHONY: all clean

all: $(LIB) $(TST)

$(LIB): CFLAGS += -fPIC
$(LIB): LDFLAGS += -shared
$(LIB): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILDDIR):
	@mkdir $@

$(TST): LDLIBS += -lbson
$(TST): $(LIB) $(TST_OBJ) 
	$(CC) $(TST_OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

rtests: $(TST)
	LD_LIBRARY_PATH=./ ./bson_test

dtests: $(TST)
	LD_LIBRARY_PATH=./ ASAN_OPTIONS=detect_leaks=0 gdb ./bson_test

clean:
	rm -r $(BUILDDIR) $(TST) $(LIB)
