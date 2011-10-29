#!/bin/awk -f

{
	line = line $0
	if(length($0) != 79) {
		print line
		line = ""
	}
}
END {
	if(line != "") {
		print line
	}
}
