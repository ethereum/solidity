# MakeFile for building all the docs at once.
# Inspired by the Makefile used by bazaar. 
# http://bazaar.launchpad.net/~bzr-pqm/bzr/2.3/
 
PYTHON = python
 
.PHONY: all clean html latexpdf epub htmlhelp
 
# Dependencies to perform before running other builds.
SPHINX_DEPENDENCIES = \
	zh/Makefile
 
# Copy-paste the english Makefile everwhere its needed.
%/Makefile: en/Makefile
	$(PYTHON) -c "import shutil; shutil.copyfile('$<', '$@')"
 
# Make the HTML version of the documentation with correctly nested language folders.
html: $(SPHINX_DEPENDENCIES)
	cd en && make html LANG=en
	cd zh && make html LANG=zh
 
htmlhelp: $(SPHINX_DEPENDENCIES)
	cd en && make htmlhelp LANG=en
	cd zh && make htmlhelp LANG=zh
 
epub: $(SPHINX_DEPENDENCIES)
	cd en && make epub LANG=en
	cd zh && make epub LANG=zh
 
latexpdf: $(SPHINX_DEPENDENCIES)
	cd en && make latexpdf LANG=en
	cd zh && make latexpdf LANG=zh
 
clean:
	rm -rf build/*
