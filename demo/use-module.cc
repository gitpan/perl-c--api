
#include <stdio.h>
#include "perl-glue.hh"

wPerl perl;

main()
{
    perl.use("lib", "./lib");
    perl.use("stuff");

#if defined(COMPACT_AND_ILLEGIBLE)

    printf("c = %d\n", perl.scalar("stuff::c").as_integer());
    printf("f() = %d\n", perl.subroutine("stuff::f")().as_integer());

#else

    wPerlScalar c = perl.scalar("stuff::c");
    wPerlScalar f = perl.subroutine("stuff::f");

    printf("c = %d\n", c.as_integer());
    printf("f() = %d\n", f().as_integer());

#endif

    return 0;
}
