PG_CONFIG = pg_config
PKG_CONFIG = pkg-config

extension_version = 1

EXTENSION = uri
MODULE_big = uri
OBJS = uri.o
DATA = uri--0--1.sql
DATA_built = uri--$(extension_version).sql

ifeq (no,$(shell $(PKG_CONFIG) liburiparser || echo no))
$(warning liburiparser not registed with pkg-config, build might fail)
endif

PG_CPPFLAGS += $(shell $(PKG_CONFIG) --cflags-only-I liburiparser)
SHLIB_LINK += $(shell $(PKG_CONFIG) --libs liburiparser)

REGRESS = init test
REGRESS_OPTS = --inputdir=test

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

uri--$(extension_version).sql: uri.sql
	cat $^ >$@
