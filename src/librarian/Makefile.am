pkginclude_HEADERS = rarian.h rarian-main.h rarian-reg-utils.h \
		rarian-info.h rarian-man.h
lib_LTLIBRARIES = librarian.la

librarian_la_SOURCES  =  \
    rarian.h             \
    rarian-main.h	\
    rarian-main.c        \
    rarian-reg-utils.h   \
    rarian-reg-utils.c	\
    rarian-language.c	\
    rarian-language.h	\
    rarian-utils.h		\
    rarian-utils.c	\
    rarian-info.h	\
    rarian-info.c	\
    rarian-man.c		\
    rarian-man.h

if ENABLE_OMF_READ
librarian_la_SOURCES  +=  	\
    rarian-omf.h		\
    rarian-omf.cpp		\
    $(top_srcdir)/util/tinyxml.cpp			\
    $(top_srcdir)/util/tinyxml.h			\
    $(top_srcdir)/util/tinyxmlparser.cpp		\
    $(top_srcdir)/util/tinystr.cpp			\
    $(top_srcdir)/util/tinystr.h			\
    $(top_srcdir)/util/tinyxmlerror.cpp	
endif

librarian_la_CFLAGS = -DDATADIR=\""$(datadir)"\" -DSYSCONFDIR=\""$(sysconfdir)"\" -DI_KNOW_RARIAN_0_8_IS_UNSTABLE
librarian_la_LDFLAGS = -export-symbols-regex "^rrn_.*"