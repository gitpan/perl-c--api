
#include <stdio.h>
#include "perl-glue.hh"

main()
{
    wPerlScalar r = 0;

    for (int i = 0; i < 100000; ++i)
    {
	r += 1;
    }

    printf("r = %d\n", r.as_integer());

    return 0;
}
