
#include <stdio.h>
#include <unistd.h>
#include "perl-glue.hh"

wPerl perl;

main()
{
    wPerlHashShadow SIG = perl.hash("SIG");
    wPerlScalar ring = perl.eval("sub {"
				    "print 'alarm went off\n';"
				 "}");

    SIG.set("ALRM", ring);
    alarm(2);
    sleep(5);

    return 0;
}
