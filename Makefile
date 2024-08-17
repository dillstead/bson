NDEBUG ?= 0
NASAN ?= 0

NAME      := bson
SLIB      := lib$(NAME).so
DLIB      := lib$(NAME).a
TST       := $(NAME)_test
EXAMPLE   := $(NAME)_example
SRC       := str.c toker.c arena.c bson_int.c bson_str.c bson_list.c \
	bson_obj.c bson.c 
TST_SRC   := test.c test_utils.c test_tok.c test_types.c test_parse.c \
	test_arena.c
EXAMPLE_SRC := example.c
SDIR      := static
SOBJ      := $(SRC:%.c=$(SDIR)/%.o)
TST_OBJ   := $(TST_SRC:%.c=$(SDIR)/%.o)
EXAMPLE_OBJ := $(EXAMPLE_SRC:%.c=$(SDIR)/%.o)
DDIR      := dynamic
DOBJ      := $(SRC:%.c=$(DDIR)/%.o)
CFLAGS    := -std=c11 -Wall -Wextra
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

all: $(SLIB) $(DLIB) $(TST) $(EXAMPLE)


$(SLIB): $(SOBJ)
	$(AR) $(ARFLAGS) $@ $^

$(DLIB): CFLAGS += -fPIC
$(DLIB): LDFLAGS += -shared
$(DLIB): $(DOBJ)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(SDIR)/%.o: %.c | $(SDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(DDIR)/%.o: %.c | $(DDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(SDIR) $(DDIR):
	@mkdir $@

$(TST): LDLIBS += -l:$(SLIB)
$(TST): $(SLIB) $(TST_OBJ) 
	$(CC) $(TST_OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

$(EXAMPLE): LDLIBS += -l:$(SLIB)
$(EXAMPLE): $(SLIB) $(EXAMPLE_OBJ) 
	$(CC) $(EXAMPLE_OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

rtests: $(TST)
	./bson_test

dtests: $(TST)
	ASAN_OPTIONS=detect_leaks=0 gdb ./bson_test

clean:
	rm -f -r $(SDIR) $(DDIR) $(SLIB) $(DLIB) $(TST) $(TST_OBJ)

