
#include <stdio.h>
#include "perl-glue.hh"

wPerl perl;

main()
{
    wPerlScalar add = perl.eval("sub {"
				   "my($a, $b) = @_;"
				   "return $a + $b;"
				"}");

    wPerlScalar x = 1;
    wPerlScalar y = 2;

    wPerlScalar r = 0;

    r = add(x, y);

    printf("r = %d\n", r.as_integer());

    return 0;
}
