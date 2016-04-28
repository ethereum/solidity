# adds possibility to run configure_file as buildstep
# reference: 
# http://www.cmake.org/pipermail/cmake/2012-May/050227.html
#
# This module expects
# INFILE
# OUTFILE
# other custom vars
#
# example usage:
# cmake -DINFILE=blah.in -DOUTFILE=blah.out -Dvar1=value1 -Dvar2=value2 -P scripts/configure.cmake 

configure_file(${INFILE} ${OUTFILE})

