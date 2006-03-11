lib_LTLIBRARIES = libin_adlib.la

libin_adlib_la_SOURCES = callback.cpp config.cpp config.h debug.h \
	dlg_about.cpp dlg_about.h dlg_config.cpp dlg_config.h dlg_info.cpp \
	dlg_info.h filetypes.cpp filetypes.h frontend.h gui_link.cpp \
	gui_link.h gui_tip.cpp gui_tip.h in2.h out.h player.cpp player.h \
	plugin.cpp plugin.h in_adlib.rc resource.h

libin_adlib_la_LDFLAGS = -no-undefined -lwinmm -lgdi32 -lcomdlg32

libin_adlib_la_LIBADD = in_adlib.lo $(adplug_LIBS)

AM_CPPFLAGS = $(adplug_CFLAGS)

EXTRA_DIST = in_adlib.dsp vc6inst.bat

LTRCCOMPILE = $(LIBTOOL) --mode=compile $(RC) $(DEFS) $(DEFAULT_INCLUDES) \
	$(INCLUDES)

.rc.lo:
	$(LTRCCOMPILE) -i $^ -o $@
