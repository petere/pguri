PG_CONFIG = pg_config
PKG_CONFIG = pkg-config

EXTENSION = uri
MODULE_big = uri
OBJS = uri.o
DATA = uri--0.sql uri--1.sql uri--0--1.sql uri--1.1.sql uri--1--1.1.sql

ifeq (no,$(shell $(PKG_CONFIG) liburiparser || echo no))
$(warning liburiparser not registed with pkg-config, build might fail)
endif

PG_CPPFLAGS += $(shell $(PKG_CONFIG) --cflags-only-I liburiparser) -Wno-int-conversion
SHLIB_LINK += $(shell $(PKG_CONFIG) --libs liburiparser)

REGRESS = init test escape
REGRESS_OPTS = --inputdir=test

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
