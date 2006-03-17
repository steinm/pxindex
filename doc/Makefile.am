
%.1: %.sgml
	$(DOC_TO_MAN) $< > $@

distclean: clean

clean:
	rm -f ${man_MANS}

EXTRA_DIST = pxindex.sgml

man_MANS = $(EXTRA_DIST:.sgml=.1)

