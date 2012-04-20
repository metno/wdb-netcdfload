.xml.1:
	$(XMLTO) man $< -o `dirname $@`

MAN_DOCS = doc/netcdfLoad.xml

man1_MANS = $(MAN_DOCS:.xml=.1)

EXTRA_DIST = $(MAN_DOCS)
CLEANFILES = $(MAN_DOCS:.xml=.1)
