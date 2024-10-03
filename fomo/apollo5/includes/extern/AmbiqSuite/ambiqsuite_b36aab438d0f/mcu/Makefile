#******************************************************************************
#
# Makefile - Rules for building the libraries, examples and docs.
#
# ${copyright}
# 
# This is part of revision ${version} of the AmbiqSuite Development Package.
#
#******************************************************************************

SUBDIRS=${wildcard */}

all:
	@for i in ${SUBDIRS};                    \
	 do                                      \
	     if [ -f $${i}/Makefile ]; then      \
	         $(MAKE) -C $${i} || exit $$?; fi;  \
	 done

clean:
	@for i in ${SUBDIRS};                    \
	 do                                      \
	     if [ -f $${i}/Makefile ]; then      \
	         $(MAKE) -C $${i} clean; fi;        \
	 done

##### INTERNAL BEGIN #####
realclean:
	@for i in ${SUBDIRS};              \
	 do                                \
	     if [ -f $${i}/Makefile ];then \
	         $(MAKE) -C $${i} realclean;  \
	     fi;                           \
	 done
##### INTERNAL END #####

	 
