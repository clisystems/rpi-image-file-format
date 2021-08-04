SUBDIRS := rpiconv rpiview

all: $(SUBDIRS)

clean:
	$(MAKE) -C rpiconv clean
	$(MAKE) -C rpiview clean

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all clean $(SUBDIRS)
