
all: perl-glue.o


C++ = g++
C++-FLAGS = -g -Wall

# If your compiler is not gcc and it does have the "bool" type and
# templates, you will need to add some -D options to your compiler flags:
#
# C++-FLAGS += -DHAS_BOOL
# C++-FLAGS += -DHAS_TEMPLATES

LINK-FLAGS = -g

PERL = perl
PERL-EMBED = -MExtUtils::Embed
PERL-INCS = `$(PERL) $(PERL-EMBED) -e ccopts`
PERL-LINKAGE = `$(PERL) $(PERL-EMBED) -e ldopts`

clean:
	/bin/rm -f \#* *.o
	(cd demo; $(MAKE) clean)
	(cd test; $(MAKE) clean)

perl-glue.o: perl-glue.cc perl-glue.hh
	$(C++) $(C++-FLAGS) $(PERL-INCS) -c -o $@ $<
