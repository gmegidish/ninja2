# Makefile.am for the SDL port of MIDAS
# See the file license.txt for licensing information for this library.

###########################################################################
#
# Some consistent rules for building asm files:

STRIP_FPIC = sh $(top_srcdir)/strip_fPIC.sh

SUFFIXES = .asm

.asm.lo:
	$(LIBTOOL) --mode=compile $(STRIP_FPIC) $(NASM) @NASMFLAGS@ $<

###########################################################################


lib_LTLIBRARIES = libmidas.la

libmidasincludedir = $(includedir)
libmidasinclude_HEADERS = midasdll.h

EXTRA_DIST = $(X86SRCS) $(HFILES) strip_fPIC.sh autogen.sh VisualC.zip

# Header files:
HFILES     = dsm.h errors.h file.h gmplayer.h it.h lang.h madpcm.h mecho.h \
             mglobals.h midas.h midasfx.h midasrc.h midasstr.h midastrk.h \
             mmem.h mpoll.h mprof.h mtypes.h mulaw.h mutils.h rawfile.h \
             sdevice.h vu.h waveread.h xm.h

# Common sources:
COMMONSRCS = mglobals.c mmem.c errors.c mutils.c file.c gmplayer.c gmpcmds.c \
             loadmod.c loads3m.c loadxm.c loadit.c dsm.c dsmmix.c dsmfilt.c \
             mix_mf.c mix_sf.c mix_mfi.c mix_sfi.c mix_mi.c mix_si.c \
             mulaw.c madpcm.c waveread.c vu.c mecho.c adpcmtbl.c \
             midasfx.c midasstr.c midas.c midasdll.c midastrk.c

SDLSRCS = rawfile.c mpoll.c midasrc.c sdl.c dsmnsnd.c

# X86-specific sources:
X86SRCS = adpcmasm.asm adsmfilt.asm adsmmix.asm amix_mf.asm amix_mfi.asm \
          amix_mi.asm amix_sf.asm amix_sfi.asm amix_si.asm cputype.asm \
          dsmmix.inc mnasm.inc mprof.inc
if HAVE_NASM
ARCHSRCS = $(X86SRCS)
else
ARCHSRCS =
endif

libmidas_la_SOURCES = $(COMMONSRCS) $(SDLSRCS) $(ARCHSRCS)

libmidas_la_LDFLAGS = 		\
        -release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	rpm -ba $(PACKAGE).spec

