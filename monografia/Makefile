BASENAME=mouse_magnetometro_monografia

all: $(BASENAME).pdf

.PHONY: all clean pvc

#$(BASENAME).pdf: $(BASENAME).tex $(BASENAME).nls
$(BASENAME).pdf: $(BASENAME).tex $(BASENAME).bib
	latexmk -pdf $< | ./filter_pdflatex_output.sed

#$(BASENAME).nls: $(BASENAME).nlo
#	makeindex $(BASENAME).nlo -s nomencl.ist -o $(BASENAME).nls

pvc:
	latexmk -pdf -pvc $(BASENAME) | ./filter_pdflatex_output.sed

clean:
	latexmk -C $(BASENAME)


# latexmk -c  will clean temporary files, but will keep: .dvi .ps .pdf
# latexmk -C  will clean temporary files and also the final files
# latexmk -pvc will run a file previewer and continuously update the final file

# I've also added these lines to ~/.latexmkrc
#$pdflatex = 'pdflatex -interaction=nonstopmode';
#
#$ps_previewer = 'start evince %O %S';
#$pdf_previewer = 'start evince %O %S';
#
#$ps_update_method = 0;
#$pdf_update_method = 0;


# If someone wants a pure-Makefile solution:
# http://snovae.in2p3.fr/ycopin/soft/Makefile.latex
#
#
# But is a lot easier to just use Latexmk:
# http://www.phys.psu.edu/~collins/software/latexmk-jcc/
#
# http://tex.stackexchange.com/questions/8317/how-to-cause-unix-make-to-know-how-many-times-to-run-tex
# http://tex.stackexchange.com/questions/64/tools-for-automating-document-compilation



# The following was my old Makefile, before discovering latexmk

#PDFLATEX=pdflatex -file-line-error -halt-on-error

#$(BASENAME).pdf: $(BASENAME).tex
# Alternate command-line:
# Don't ask me anything, just abort it!
#yes x | pdflatex $<

# Yet another alternate command-line:
#$(PDFLATEX) $< | ./filter_pdflatex_output.sed
# Running twice because of TOC
#$(PDFLATEX) $<

#clean:
# This expansion works in bash, but not in dash.
# rm -f $(BASENAME).{aux,dvi,log,nav,out,pdf,snm,toc,vrb}
# So... Let's use a simple for loop instead
# for ext in aux bbl blg dvi glg glo gls idx ilg ind lof lot log nav out pdf snm toc vrb fdb_latexmk ; do \
# 	rm -f $(BASENAME).$${ext} ; \
# done

