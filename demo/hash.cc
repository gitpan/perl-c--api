
#include <stdio.h>
#include "perl-glue.hh"

void dump_hash(wPerlHash &h)
{
    char *key;
    wPerlScalar value;

    h.reset();

    printf("$hash = {\n");
    while (h.each(&key, &value))
    {
	printf("   '%s' => '%s',\n", key, value.as_string());
    }
    printf("};\n");
}

main()
{
    wPerlHash h;

    if (!h)
    {
	printf("hash is empty\n");
    }

    h.set("alpha", 1);
    h.set("beta", 2);
    dump_hash(h);

    printf("alpha = %d\n", h["alpha"].as_integer());

    h.set("beta", "example #2");

    printf("beta = %s\n", h["beta"].as_string());

    if (!h["bogus"].defined())
    {
	printf("'bogus' is not in the hash\n");
    }

    return 0;
}
