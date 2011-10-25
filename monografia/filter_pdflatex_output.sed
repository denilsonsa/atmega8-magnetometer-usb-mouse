#!/bin/sed -f

# Lines with the paths to the loaded packages
/^[() ]*[(<{]\/usr\/share\/tex/d

# Lines with just numbers or empty lines
/^[][()0-9 ]*$/d

# One .aux with lots of fonts
/^([^)>}]*\.aux) ){[^}>)].*}\(<[^>)}]*\.pfb>\)+$/d

# Included file being printed for every page, like this:
# <use shared/logo.pdf> [11] <use shared/logo.pdf> [12]
# Of course, it only matches things from the "shared" subdir
/^\(<use shared\/[^>]*> \[[0-9]*\] *\)+$/d

# Yeah, everybody knows there is a log file, no need to keep this line
/^Transcript written on [^ ]*\.log\.$/d
