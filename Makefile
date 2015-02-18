PG_CONFIG = pg_config
PKG_CONFIG = pkg-config

extension_version = 0

EXTENSION = pguri
MODULE_big = pguri
OBJS = pguri.o
DATA_built = pguri--$(extension_version).sql

ifeq (no,$(shell $(PKG_CONFIG) liburiparser || echo no))
$(warning liburiparser not registed with pkg-config, build might fail)
endif

PG_CPPFLAGS += $(shell $(PKG_CONFIG) --cflags-only-I liburiparser)
SHLIB_LINK += $(shell $(PKG_CONFIG) --libs liburiparser)

REGRESS = init test
REGRESS_OPTS = --inputdir=test

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

pguri--$(extension_version).sql: pguri.sql
	cat $^ >$@
