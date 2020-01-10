INCLUDES =      \
    -I$(srcdir)/../librarian
LDADD =         \
    ../librarian/librarian.la

if ENABLE_SK_COMPAT
bin_SCRIPTS = \
	rarian-sk-update \
	rarian-sk-install \
	rarian-sk-rebuild \
	rarian-sk-config  \
	rarian-sk-get-scripts \
	rarian-sk-get-content-list \
	rarian-sk-get-extended-content-list
endif

DISTCLEANFILES= rarian-sk-update \
	rarian-sk-install \
	rarian-sk-rebuild \
	rarian-sk-config  \
	rarian-sk-get-scripts \
	rarian-sk-get-content-list \
	rarian-sk-get-extended-content-list

bin_PROGRAMS = \
    rarian-example

if ENABLE_SK_COMPAT
bin_PROGRAMS += rarian-sk-migrate \
    rarian-sk-preinstall \
    rarian-sk-gen-uuid \
    rarian-sk-get-cl

if ENABLE_EXTRACT
bin_SCRIPTS += rarian-sk-extract
endif
endif

if ENABLE_SK_COMPAT
rarian_sk_migrate_SOURCES =  \
    rarian-sk-migrate.cpp    \
    tinystr.cpp             \
    tinyxml.cpp             \
    tinyxmlerror.cpp        \
    tinyxmlparser.cpp       \
    tinystr.h               \
    tinyxml.h               \
    ../librarian/rarian-reg-full.h \
    ../librarian/rarian-reg-full.c

rarian_sk_preinstall_SOURCES = \
    rarian-sk-preinstall.cpp \
    tinystr.cpp             \
    tinyxml.cpp             \
    tinyxmlerror.cpp        \
    tinyxmlparser.cpp       \
    tinystr.h               \
    tinyxml.h

rarian_sk_get_cl_SOURCES =  \
    rarian-sk-get-cl.cpp    \
    tinystr.cpp             \
    tinyxml.cpp             \
    tinyxmlerror.cpp        \
    tinyxmlparser.cpp       \
    tinystr.h               \
    tinyxml.h

rarian_sk_get_cl_CXXFLAGS = -DDATADIR=\""$(datadir)"\" -DLOCALSTATEDIR=\""$(localstatedir)"\"

rarian_sk_gen_uuid_SOURCES = \
    rarian-sk-gen-uuid.c
endif

rarian_example_SOURCES =  \
    rarian-example.c

if ENABLE_INSTALL
install-exec-hook:
if ENABLE_SK_COMPAT
	@echo Moving Files if needed
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-config ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-config; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-config ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-config $(DESTDIR)$(bindir)/scrollkeeper-config.real; fi
if ENABLE_EXTRACT
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-extract ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-extract; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-extract ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-extract $(DESTDIR)$(bindir)/scrollkeeper-extract.real; fi
endif
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-gen-seriesid ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-gen-seriesid; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-gen-seriesid ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-gen-seriesid $(DESTDIR)$(bindir)/scrollkeeper-gen-seriesid.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-cl ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-cl; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-cl ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-cl $(DESTDIR)$(bindir)/scrollkeeper-get-cl.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-content-list ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-content-list; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-content-list ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-content-list $(DESTDIR)$(bindir)/scrollkeeper-get-content-list.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-extended-content-list ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-extended-content-list; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-extended-content-list ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-extended-content-list $(DESTDIR)$(bindir)/scrollkeeper-get-extended-content-list.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-index-from-docpath ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-index-from-docpath; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-index-from-docpath ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-index-from-docpath $(DESTDIR)$(bindir)/scrollkeeper-get-index-from-docpath.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-docpath ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-docpath; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-docpath ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-docpath $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-docpath.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-id ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-id; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-id ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-id $(DESTDIR)$(bindir)/scrollkeeper-get-toc-from-id.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-install ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-install; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-install ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-install $(DESTDIR)$(bindir)/scrollkeeper-install.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-preinstall ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-preinstall; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-preinstall ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-preinstall $(DESTDIR)$(bindir)/scrollkeeper-preinstall.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-rebuilddb ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-rebuilddb; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-rebuilddb ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-rebuilddb $(DESTDIR)$(bindir)/scrollkeeper-rebuilddb.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-uninstall ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-uninstall; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-uninstall ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-uninstall $(DESTDIR)$(bindir)/scrollkeeper-uninstall.real; fi
	@if [ -h $(DESTDIR)$(bindir)/scrollkeeper-update ]; then rm $(DESTDIR)$(bindir)/scrollkeeper-update; \
	elif [ -e $(DESTDIR)$(bindir)/scrollkeeper-update ]; then \
		mv $(DESTDIR)$(bindir)/scrollkeeper-update $(DESTDIR)$(bindir)/scrollkeeper-update.real; fi
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-config scrollkeeper-config
if ENABLE_EXTRACT
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-extract scrollkeeper-extract
endif
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-gen-uuid scrollkeeper-gen-seriesid
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-cl scrollkeeper-get-cl
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-content-list scrollkeeper-get-content-list
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-extended-content-list scrollkeeper-get-extended-content-list
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-scripts scrollkeeper-get-index-from-docpath
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-scripts scrollkeeper-get-toc-from-docpath
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-get-scripts scrollkeeper-get-toc-from-id
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-install scrollkeeper-install
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-preinstall scrollkeeper-preinstall
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-rebuild scrollkeeper-rebuilddb
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-install scrollkeeper-uninstall
	@cd $(DESTDIR)$(bindir); ln -s rarian-sk-update scrollkeeper-update
	@mkdir -p $(DESTDIR)/$(localstatedir)/lib/rarian
if ENABLE_SKDB_UPDATE
	@echo "Translating current database"
	@$(DESTDIR)$(bindir)/rarian-sk-update
endif
endif

else
install-exec-hook:
	@echo WARNING: Installed in UNINSTALLED mode.  Prepare for things to be broken!!!!!
endif
