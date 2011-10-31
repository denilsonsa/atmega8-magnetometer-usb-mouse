#!/bin/sed -f

# Lines with the paths to the loaded packages
/^[() ]*[(<{]\/usr\/share\/tex/d

# Lines with just numbers or empty lines
/^[][()0-9 ]*$/d

# One .aux with lots of fonts
/^([^)>}]*\.aux) ){[^}>)].*}\(<[^>)}]*\.pfb>\)+$/d

# Included file being printed, like this:
# <use img/sensor_front.jpg>
# <use /home/denilson/avr/PDFs/doc2486.pdf, page 158>
# <use /home/denilson/avr/PDFs/doc2486.pdf, page 158> [28 </home/denilson/avr/PDFs/doc2486.pdf>]
# <use shared/logo.pdf> [11] <use shared/logo.pdf> [12]
/^\(\(<use [^>]*\.\(pdf\|png\|jpg\)\(, page [0-9]\+\)\?>\)\?\( *\[[0-9]\+\( *<[^>]*>\)\?\] *\)\?\)\+$/d

# More included files:
# [40] <../../hmc5883/HMC5883L.pdf, id=721, page=1, 614.295pt x 794.97pt>
# <img/circuito-USBasp.pdf, id=430, 240.9pt x 349.305pt>
# </home/denilson/avr/PDFs/doc2564.pdf, id=1287, page=1, 597.45204pt x 845.1575pt> [104]
/^\(\(\[[0-9]\+\]\)\? *<[^>]*\.\(pdf\|png\|jpg\), id=[^>]*> *\(<use [^>]*\.\(pdf\|png\|jpg\)\(, page [0-9]\+\)\?>\)\? *\(\[[0-9]\+\]\)\? *\)\+$/d

# Yeah, everybody knows there is a log file, no need to keep this line
/^Transcript written on [^ ]*\.log\.$/d

# I'm aware of this warning. I don't care.
/^pdfTeX warning: pdflatex (file [^)]*): PDF inclusion: found PDF version <1.6>, but at most version <1.5> allowed/d

# I'm aware of this warning. I don't care.
/^Package hyperref Warning: Token not allowed in a PDF string (PDFDocEncoding)/d
/^(hyperref)[ \t]*removing `.*' on input line/d
