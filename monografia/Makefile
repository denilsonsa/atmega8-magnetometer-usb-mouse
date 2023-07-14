BASENAME=mouse_magnetometro_monografia

all: $(BASENAME).pdf

.PHONY: all clean pvc

doc2486.pdf:
	@echo If the automatic download fails, try manually downloading it from:
	@echo 'https://www.microchip.com/en-us/product/ATmega8#document-table'
	curl -f 'https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2486-8-bit-AVR-microcontroller-ATmega8_L_datasheet.pdf' -o doc2486.pdf

doc2564.pdf:
	@echo If the automatic download fails, try manually downloading it from:
	@echo 'https://www.microchip.com/en-us/product/ATmega8#document-table'
	curl -f 'https://ww1.microchip.com/downloads/en/Appnotes/00002480A.pdf' -o doc2564.pdf

HMC5883L.pdf:
	curl -f 'https://cdn-shop.adafruit.com/datasheets/HMC5883L_3-Axis_Digital_Compass_IC.pdf' -o HMC5883L.pdf

en026368.pdf:
	curl -f 'https://ww1.microchip.com/downloads/en/DeviceDoc/chapter%208.pdf' -o en026368.pdf

AN97055.pdf:
	curl -f 'http://www.kip.uni-heidelberg.de/lhcb/Publications/external/AN97055.pdf' -o AN97055.pdf

AN10441.pdf:
	curl -f 'https://files.seeedstudio.com/wiki/Grove-I2C_High_Accuracy_Temperature_Sensor-MCP9808/res/AN10441.pdf' -o AN10441.pdf

$(BASENAME).pdf: $(BASENAME).tex $(BASENAME).bib doc2486.pdf HMC5883L.pdf en026368.pdf AN97055.pdf AN10441.pdf
	latexmk -pdf -file-line-error -halt-on-error $< | ./join_lines.awk | ./filter_pdflatex_output.sed

#$(BASENAME).nls: $(BASENAME).nlo
#	makeindex $(BASENAME).nlo -s nomencl.ist -o $(BASENAME).nls

pvc:
	latexmk -pdf -file-line-error -halt-on-error -pvc $(BASENAME) | ./join_lines.awk | ./filter_pdflatex_output.sed

clean:
	latexmk -C $(BASENAME)
	rm -f $(BASENAME).bbl


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

