
#include <stdio.h>
#include "perl-glue.hh"

main()
{
    wPerl perl;
#if 0
    wPerlScalar add = perl.eval("sub {"
				    "$_[0] + $_[1];"
				"}");
#endif
    wPerlScalar r = 0;
    wPerlScalar s = 0;
    wPerlScalar t;

    for (int i = 0; i < 10000; ++i)
    {
	r += i;
    }

    printf("r = %d\n", r.as_integer());

    if (r.eq("49995000"))
    {
	printf("yes!\n");
    }

    if (r) printf("r ok!\n");
    if (s) printf("s ok!\n");
    if (t) printf("t ok!\n");

    wPerlArray a;
    if (a) printf("a ok!\n");
    a.set(0, 1);
    if (a) printf("a ok!\n");

    wPerlHash h;
    if (h) printf("h ok!\n");
    h.set(0, 1);
    if (h) printf("h ok!\n");

    perl.eval("%hash = ();");
    wPerlHashShadow hash = perl.hash("hash");

    hash.set("ALRM", 1);

    perl.eval("print 'hash ALRM = ', $hash{ALRM}, ' %hash = ', \\%hash, '\n'");
    printf("(from C) hash ALRM = %s %%hash = %x\n", hash.get("ALRM").as_string(), hash.v);

    wPerlHashShadow SIG = perl.hash("SIG");
    if (SIG) printf("SIG ok!\n");

    perl.eval("print 'SIG ALRM = ', $SIG{ALRM}, ' %SIG = ', \\%SIG, '\n'");
    printf("(from C) SIG ALRM = %s %%SIG = %x\n", SIG.get("ALRM").as_string(), SIG.v);

    SIG.set("ALRM", perl.eval("sub { print 'alarm went off (1)\n' }"));

    perl.eval("print 'SIG ALRM = ', $SIG{ALRM}, ' %SIG = ', \\%SIG, '\n'");
    printf("(from C) SIG ALRM = %s %%SIG = %x\n", SIG.get("ALRM").as_string(), SIG.v);

    alarm(2);
    printf("time = %ld\n", time(0));
    sleep(5);
    printf("time = %ld\n", time(0));

    perl.eval("$SIG{'ALRM'} = sub { print 'alarm went off (2)\n' }");

    perl.eval("print 'SIG ALRM = ', $SIG{ALRM}, ' %SIG = ', \\%SIG, '\n'");
    printf("(from C) SIG ALRM = %s %%SIG = %x\n", SIG.get("ALRM").as_string(), SIG.v);

    alarm(2);
    printf("time = %ld\n", time(0));
    sleep(5);
    printf("time = %ld\n", time(0));

    r.append(" cm").append(" per sec");
    printf("r = %s\n", r.as_string());

    exit(0);
}
