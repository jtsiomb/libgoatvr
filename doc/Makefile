rstfiles = $(wildcard *.rst)
htmlfiles = $(rstfiles:.rst=.html)

.PHONY: html
html: $(htmlfiles)

%.html: %.rst
	rst2html $(RSTFLAGS) $< >$@

.PHONY: clean
clean:
	rm -f $(htmlfiles)
