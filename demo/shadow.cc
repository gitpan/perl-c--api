
#include <stdio.h>
#include "perl-glue.hh"

wPerl perl;

main()
{
    wPerlScalar y = 0;
    wPerlScalarShadow x = y;

    x = 2;
    printf("x = %d, y = %d\n", x.as_integer(), y.as_integer());

    y = 3;
    printf("x = %d, y = %d\n", x.as_integer(), y.as_integer());

    wPerlScalarShadow z = perl.scalar("z", wPerl::Create);

    z = 10;
    perl.eval("print \"z = $z\n\"");

    perl.eval("$z = 20");
    printf("z = %d\n", z.as_integer());

    return 0;
}
