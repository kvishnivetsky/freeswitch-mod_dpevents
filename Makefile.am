include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_dpevents

mod_LTLIBRARIES = mod_dpevents.la
mod_dpevents_la_SOURCES  = mod_dpevents.c
mod_dpevents_la_CFLAGS   = $(AM_CFLAGS)
mod_dpevents_la_LIBADD   = $(switch_builddir)/libfreeswitch.la
mod_dpevents_la_LDFLAGS  = -avoid-version -module -no-undefined -shared
