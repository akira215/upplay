.SUFFIXES: .txt .html

.txt.html:
	asciidoc $<

all: upplay.html upplay-releases.html upplay-mingw.html \
     install.html songcast.html

clean:
	rm -f *.html
