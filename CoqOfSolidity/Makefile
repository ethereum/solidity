.PHONY: all clean clean-util distclean default

VFILES := $(shell find . -name "*.v")

default: all

CoqMakefile: $(VFILES)
	echo "-R . CoqOfSolidity" > _CoqProject
	echo "-arg -impredicative-set" >> _CoqProject
	find . -name "*.v" | sort >> _CoqProject
	coq_makefile -f _CoqProject -o $@

MAKECOQ := +$(MAKE) -f CoqMakefile

%.vo: CoqMakefile %.v
	$(MAKECOQ) $@

all: CoqMakefile
	$(MAKECOQ) all

clean-coq: CoqMakefile
	$(MAKECOQ) clean

clean-util:
	rm -f *CoqMakefile*

clean: clean-coq
	$(MAKE) clean-util # done separately to enforce order

distclean: clean
