
#include <stdio.h>
#include "perl-glue.hh"

main()
{
    if (!wPerl::run()->use("Sys::Hostname"))
    {
	printf("couldn't load Sys::Hostname\n");
	exit(1);
    }

    wPerlScalar hostname = wPerl::run()->subroutine("hostname");

    printf("hostname = %s\n", hostname().as_string());

    return 0;
}
