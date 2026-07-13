SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = src
BUILDDIR      = build


.PHONY: doc

doc:
	rm -rf doc_out
	rm -rf doc/*
	mkdir -p doc
	doxygen configs/Doxyfile.in
	moxygen -c  -a -o doc/%s.md doc_out/xml/