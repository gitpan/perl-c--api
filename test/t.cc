
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "perl-glue.hh"

int tests_passed;
int tests_asserted;

#define test_assertion(E, M) \
    ++tests_asserted; \
    if (!(E)) { \
	printf("at line %d: " #E " failed: ", __LINE__); printf M; printf("\n"); \
    } \
    else { \
	printf(#E " passed\n"); \
	++tests_passed; \
    }

wPerl perl;

void test_eval()
{
    wPerlScalar n = perl.eval("10 + 5;");

    if (n.defined())
    {
	test_assertion(n.as_integer() == 15, ("eval returned wrong result"));
	test_assertion(strcmp(n.as_string(), "15") == 0, ("eval returned wrong result"));
	double x = n.as_real();
	test_assertion(x - 0.0001 < 15.0 && x + 0.0001 > 15.0, ("eval returned wrong result %g", x));
    }
    else
    {
	printf("scalar not defined -- eval failed?\n");
	fflush(stdout);
	abort();
    }

    wPerlScalar add = perl.eval("sub {"
				    "my($a, $b) = @_;"
				    "$a + $b;"
				"}");
    wPerlScalar sum = add(3, 7);
    test_assertion(sum.as_integer() == 10, ("call returned wrong result"));
    sum = add(3.5, 7);
    test_assertion(sum.as_real() - 0.0001 < 10.5 && sum.as_real() + 0.0001 > 10.5, ("call returned wrong result"));

    // perl might be a bit too lenient in letting this go by...
    sum = add(1);
    test_assertion(sum.as_integer() == 1, ("call returned wrong result"));
}

bool bound(const char *name, wPerlScalar &value)
{
    if (value.defined())
    {
	int the_external_value;
	int the_internal_value;
	int the_new_value;

	the_external_value = perl.eval(name).as_integer();
	the_internal_value = value.as_integer();
	if (the_external_value != the_internal_value) return false;

	the_new_value = the_internal_value + 1;
	value = the_new_value;

	the_external_value = perl.eval(name).as_integer();
	the_internal_value = value.as_integer();

	if (the_internal_value != the_new_value)
	{
	    printf("integer value assignment not working properly\n");
	    fflush(stdout);
	    abort();
	}

	if (the_external_value != the_internal_value) return false;

	return true;
    }

    printf("%s not defined internally\n", name);

    return false;
}

wPerlScalar as_var(char *name)
{
    return perl.scalar(name);
}

wPerlScalarShadow as_ref(char *name)
{
    return perl.scalar(name);
}

void test_binding()
{
    wPerlScalar X = perl.scalar("X");
    test_assertion(!bound("$X", X), ("internal X should not be bound to $X"));

    wPerlScalar Y = 0;
    Y = perl.scalar("Y");
    test_assertion(!bound("$Y", Y), ("internal Y should not be bound to $Y"));

    wPerlScalarShadow X_bound = perl.scalar("X");
    test_assertion(bound("$X", X_bound), ("internal X_bound should be bound to $X"));

    X_bound = 99;
    test_assertion(X_bound.as_integer() == 99, ("shadow variable isn't working"));
    test_assertion(perl.scalar("X").as_integer() == 99, ("shadow variable isn't working"));

    wPerlScalar Z = X_bound;
    test_assertion(!bound("$X", Z), ("internal Z should not be bound to $X"));

    X_bound = Y;
    X_bound = 10;
    Y = 11;
    printf("*** X = %d\n", X_bound.as_integer());
    test_assertion(X_bound.as_integer() == 10, ("internal Y should not be bound to X"));
    test_assertion(Y.as_integer() == 11, ("internal Y should not be bound to X"));

    Y = X_bound;
    X_bound = 11;
    test_assertion(X_bound.as_integer() == 11 && Y.as_integer() == 10, ("internal Y should not be bound to X"));

    perl.scalar("X") = 27;
    Y = X_bound;
    test_assertion(Y.as_integer() == 27, ("direct assignment to $X failed"));
    test_assertion(perl.scalar("X").as_integer() == 27, ("direct assignment to $X failed"));

    wPerlScalar Possible_Optimizer_Bug1 = as_var("X");
    test_assertion(!bound("$X", Possible_Optimizer_Bug1), ("internal variable should not be bound to $X"));

    wPerlScalarShadow Possible_Optimizer_Bug2 = as_var("X");
    test_assertion(!bound("$X", Possible_Optimizer_Bug2), ("internal variable should not be bound to $X"));

    wPerlScalar Possible_Optimizer_Bug3 = as_ref("X");
    test_assertion(!bound("$X", Possible_Optimizer_Bug3), ("internal variable should not be bound to $X"));

    wPerlScalarShadow Possible_Optimizer_Bug4 = as_ref("X");
    test_assertion(bound("$X", Possible_Optimizer_Bug4), ("internal variable should be bound to $X"));
}

void test_searching()
{
    wPerlScalar s1 = "This is a test of the Perl/C++ interface.";

    test_assertion(s1.find("Perl"), ("string should have been found"));
    test_assertion(s1.find("is a"), ("string should have been found"));
    test_assertion(!s1.find("is not a"), ("string should not have been found"));
    test_assertion(!s1.find("PERL"), ("string should not have been found"));

    wPerlArray matches;
    test_assertion(s1.find("a\\s+(\\w+)", &matches), ("pattern should have been found"));
    test_assertion(matches.length() == 1, ("pattern should have identified 1 match"));
    test_assertion(strcmp(matches.get(0).as_string(), "test") == 0, ("pattern should have identified 'test'"));
    test_assertion(s1.find_all("[aeiou]") == 11, ("pattern should have been found 11 times"));
    test_assertion(s1.find_all("[aeiou]", &matches) == 11, ("pattern should have been found 11 times"));
    test_assertion(matches.length() == 11, ("pattern should have identified 11 matches"));
    test_assertion(strcmp(matches.get(0).as_string(), "i") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(1).as_string(), "i") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(2).as_string(), "a") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(3).as_string(), "e") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(4).as_string(), "o") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(5).as_string(), "e") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(6).as_string(), "e") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(7).as_string(), "i") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(8).as_string(), "e") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(9).as_string(), "a") == 0, ("pattern should match"));
    test_assertion(strcmp(matches.get(10).as_string(), "e") == 0, ("pattern should match"));

    test_assertion(s1.find_all(".") == 41, ("string should have 41 characters"));
    test_assertion(s1.find_all(".", &matches) == 41, ("string should have 41 characters"));
    {
	char *ptr = s1.as_string();
	int i = 0;
	int ok = 0;
	while (*ptr)
	{
	    if (*ptr == *(matches[i].as_string())) ++ok;
	    ++ptr;
	    ++i;
	}
	test_assertion(ok == 41, ("letter by letter comparison fails"));
    }

    int count = 0;
    s1.restart_search();
    while (s1.find_next("\\w+")) ++count;
    test_assertion(count == 9, ("should have iterated through 9 words, only did %d", count));
    count = 0;
    s1.restart_search();
    while (s1.find_next("\\w+")) ++count;
    test_assertion(count == 9, ("should have iterated through 9 words, only did %d", count));
    count = 0;
    while (s1.find_next("\\w+")) ++count;
    test_assertion(count == 0, ("should have iterated through 0 words"));

    static char *matched_words[] = { "This", "is", "a", "test", "of", "the", "Perl", "C", "interface" };
    count = 0;
    s1.restart_search();
    while (s1.find_next("(\\w+)", &matches))
    {
	test_assertion(matches.length() == 1, ("pattern should have identified a match"));
	test_assertion(strcmp(matches.get(0).as_string(), matched_words[count]) == 0, ("pattern should match %s", matched_words[count]));
	++count;
    }
    test_assertion(count == 9, ("should have iterated through 9 words, only did %d", count));

    s1.restart_search();

    wPerlPattern regexp("/\\w+ \\w+/");
    test_assertion(s1.apply_pattern(regexp), ("should have found two words together"));
    wPerlScalar s2 = ")*#(*&#%(*$(%*&$(*&(*&$(%*&$(%*&$(*@#$_(*(_@#$_)(#%";
    test_assertion(!s2.apply_pattern(regexp), ("should not have found two words together"));

    regexp = "/(\\w+)/g";
    count = 0;
    while (s1.apply_pattern(regexp, &matches))
    {
	test_assertion(matches.length() == 1, ("pattern should have identified a match"));
	test_assertion(strcmp(matches.get(0).as_string(), matched_words[count]) == 0, ("pattern should match %s", matched_words[count]));
	++count;
    }
    test_assertion(count == 9, ("should have iterated through 9 words, only did %d", count));

    s1.restart_search();
}

void test_replacing()
{
    wPerlScalar s1 = "This is a test of the Perl/C++ interface.";
    wPerlScalar s2 = "THIS IS A TEST OF THE PERL/C++ INTERFACE.";
    wPerlScalar s2_copy = s2;
    wPerlScalar s3 = "TH<eye>S <eye>S A TEST OF THE PERL/C++ <eye>NTERFACE.";
    wPerlPattern regexp("s/(\\w)/\\U$1/g");
    wPerlArray matches;

    wPerlScalar eq = perl.eval("sub {"
				 "my($a, $b) = @_;"
				 "$a eq $b;"
			       "}");

    test_assertion(!eq(s1, s2).as_integer(), ("strings should not be equal"));
    test_assertion(s1.apply_pattern(regexp, &matches), ("should have replaced lowercase letters"));
    test_assertion(eq(s1, s2).as_integer(), ("strings should be equal"));

    test_assertion(!eq(s2, s3).as_integer(), ("strings should not be equal"));
    test_assertion(s2.replace("I", "<eye>"), ("should have replaced 'I'"));

    test_assertion(eq(s2, s3).as_integer(), ("strings should be equal"));
    test_assertion(!s2.replace("I", "<eye>"), ("should not have replaced 'I'"));
    test_assertion(eq(s2, s3).as_integer(), ("strings should be equal"));

    wPerlScalar s4 = s1.substr(5, 2);
    test_assertion(eq(s4, "IS").as_integer(), ("strings should be equal"));
    s4 = "no affect on original";
    test_assertion(eq(s1, s2_copy).as_integer(), ("strings should be equal"));
    s4 = s2_copy;
    s4.replace_substr("(Replaced 'is' with this)", 6, 2);
    test_assertion(!eq(s4, s2_copy).as_integer(), ("strings should not be equal"));
    test_assertion(!eq(s4, "THIS (Replaced 'is' with this) A TEST OF THE PERL/C++ INTERFACE.").as_integer(), ("strings should be equal"));
}

void test_arrays()
{
    wPerlArray a;
    wPerlScalar dump = perl.eval("sub { print 'array = ', join(', ', @{$_[0]}), '\n'; }");

    a.push(10); dump(a);
    a.push(20); dump(a);
    a.push(30); dump(a);

    a.pop();    dump(a);
    a.pop();    dump(a);
    a.pop();    dump(a);

    wPerlScalar e = a.pop(); dump(a);

    if (!e) printf("array is empty\n");
    if (!a) printf("array is empty\n");

    a.push(1);	dump(a);
    a.push(2);	dump(a);
    a.push(3);	dump(a);

    wPerlScalar x, y;

    a >> x >> y;
    dump(a);

    printf("x = %d, y = %d\n", x.as_integer(), y.as_integer());

    a << y << x;
    dump(a);

    printf("length = %d\n", a.length());

    short s = 0;
    a.unshift(s); dump(a);
    a.unshift_as_integer(1L); dump(a);
    float f = 2.5;
    a.unshift(f); dump(a);
    a.unshift_as_real(f); dump(a);
    a.unshift_as_bytes("stuff", 5); dump(a);

    wPerlArray b;
    b = a;

    printf("B: "); dump(b);

    while (a)
    {
	printf("length = %d\n", a.length());
	a.shift(); dump(a);
    }

    printf("length = %d\n", a.length());

    printf("B: "); dump(b);

    wPerlScalar zzz = 10;

    a.set(0, zzz);
    a.set_shared(1, zzz);
    a.set(2, zzz);
    a.set_shared(3, zzz);
    a.set_shared(4, zzz);
    dump(a);

    zzz = 20;
    dump(a);

    a.set(3, 15);
    dump(a);
}

void test_hashes()
{
    wPerlHash h;

    h.set("alpha", 10);
    h.set("beta", 20);
    h.set("gamma", 30);

    h.set(10, 100);
    h.set(20, 200);
    h.set(30, 300);

    struct point {
	int x, y;
    };

    point p;
    p.x = 10;
    p.y = 20;

    h.set(&p, sizeof(p), 10000);

    test_assertion(h.exists("alpha"), ("'alpha' should exist in hash"));
    test_assertion(!h.exists("rho"), ("'rho' should not exist in hash"));

    test_assertion(h.get("alpha").as_integer() == 10, ("hash lookup failed"));
    test_assertion(!h.get("rho").defined(), ("hash lookup should have failed"));
    test_assertion(h.get("rho").as_integer() == -1, ("hash lookup should have failed"));

    test_assertion(h.get(20).as_integer() == 200, ("hash lookup failed"));
    test_assertion(!h.get(40).defined(), ("hash lookup should have failed"));
    test_assertion(h.get(40).as_integer() == -1, ("hash lookup should have failed"));

    point q;
    q.x = 10;
    q.y = 20;
    test_assertion(h.get(&q, sizeof(q)).as_integer() == 10000, ("hash lookup failed"));
    q.y = 30;
    test_assertion(h.get(&q, sizeof(q)).as_integer() == -1, ("hash lookup should have failed"));

    h.set_as_bytes("p1", &q, sizeof(q));
    q.y = 300;
    h.set_as_bytes("p2", &q, sizeof(q));

    point *ptr;

    ptr = (point *)h.get("p1").as_bytes();
    test_assertion(ptr != 0, ("hash lookup failed"));
    test_assertion(ptr->y == 30, ("hash lookup failed"));

    ptr = (point *)h.get("p2").as_bytes();
    test_assertion(ptr != 0, ("hash lookup failed"));
    test_assertion(ptr->y == 300, ("hash lookup failed"));

    ptr = (point *)h.get("p3").as_bytes();
    test_assertion(ptr == 0, ("hash lookup failed"));

    h.set("alpha", 48);
    test_assertion(h.get("alpha").as_integer() == 48, ("hash lookup failed"));

    h.remove("alpha");
    test_assertion(!h.get("alpha").defined(), ("hash lookup should have failed"));
    test_assertion(h.get("alpha").as_integer() == -1, ("hash lookup should have failed"));

    wPerlHash H = perl.hash("H");
    wPerlScalar X = H.get("beta");
    test_assertion(!X.defined(), ("internal X should not be defined"));
    test_assertion(!bound("$H{beta}", X), ("internal X should not be bound to $H{beta}"));

    H.set("beta", "b");
    wPerlScalar Y = H.get("beta");
    test_assertion(!bound("$H{beta}", Y), ("internal Y should not be bound to $H{beta}"));

    wPerlScalar tuit = perl.eval("sub {"
				    "my($h, $k) = @_;"
				    "return $h->{$k};"
				"}");

    test_assertion(tuit(h, "beta").as_integer() == 20, ("hashes not passed properly in call()"));
    test_assertion(!tuit(h, 20).defined(), ("integer hash keys are should be raw (i.e not strings)"));
    int i = 20;
    test_assertion(tuit(h, wPerlScalar(&i, sizeof(i))).as_integer() == 200, ("integer hash keys are should be raw (i.e not strings)"));

    tPerlHashR<double> hash_of_reals;
    hash_of_reals.set("1", 10.1);
    hash_of_reals.set(2,   10.7);

    test_assertion(hash_of_reals.get("1") == 10.1, ("hash of reals broken"));
    test_assertion(hash_of_reals.get(2) == 10.7, ("hash of reals broken"));
    test_assertion(hash_of_reals.get(4) == -1, ("hash of reals broken"));

    tPerlHashI<int> hash_of_integers;
    hash_of_integers.set("1", 1);
    hash_of_integers.set(2,   10);

    test_assertion(hash_of_integers.get("1") == 1, ("hash of integers broken"));
    test_assertion(hash_of_integers.get(2) == 10, ("hash of integers broken"));
    test_assertion(hash_of_integers.get(4) == -1, ("hash of integers broken"));

    tPerlHashI<char> hash_of_chars;
    hash_of_chars.set("1", 'A');
    hash_of_chars.set(2,   'b');

    test_assertion(hash_of_chars.get("1") == 'A', ("hash of chars broken"));
    test_assertion(hash_of_chars.get(2) == 'b', ("hash of chars broken"));
    test_assertion(hash_of_chars.get(4) == -1, ("hash of chars broken"));

    wPerlScalar f = perl.eval("sub {"
				 "my($h) = @_;"
				 "foreach $k (keys %$h) {"
				    "print 'key = ', $k, ' length(key) = ', length($k), '\n';"
				    "print '\t value = ', $h->{$k}, '\n';"
				 "}"
			      "}");

    printf("sub = %s\n", f.as_string());

    f(hash_of_reals);
    f(hash_of_integers);
    f(hash_of_chars);

}

void g1(wPerlScalar x)
{
    x = 101;
}

void g2(wPerlScalarShadow x)
{
    x = 102;
}

void h1(wPerlScalar &x)
{
    x = 201;
}

void h2(wPerlScalarShadow &x)
{
    x = 202;
}

void test_arithmetic()
{
    wPerlScalar a = 10;

    wPerlScalar b = 2 + a;
    test_assertion(b.as_integer() == 12, ("addition failed"));
    a += 10;
    test_assertion(a.as_integer() == 20, ("addition failed"));
    a += a;
    test_assertion(a.as_integer() == 40, ("addition failed"));

    g1(a);
    test_assertion(a.as_integer() == 40, ("pass by value failed"));
    g2(a);
    test_assertion(a.as_integer() == 102, ("pass by reference failed"));

    wPerlScalarShadow c = perl.scalar("c", wPerl::Create);
    test_assertion(!c.defined(), ("perl variable should be undefined"));

    c = 18764;

    test_assertion(c.as_integer() == 18764, ("shadow variable isn't working"));
    test_assertion(perl.scalar("c").as_integer() == 18764, ("shadow variable isn't working"));

    wPerlScalarShadow d = perl.scalar("c", wPerl::Create);
    test_assertion(d.defined(), ("perl variable should be defined"));
    test_assertion(d.as_integer() == 18764, ("variable lookup/create isn't working"));

    a = 1989;
    test_assertion(a.as_integer() == 1989, ("assignment failed failed"));
    g1(a);
    test_assertion(a.as_integer() == 1989, ("pass by value failed"));

    g2(a);
    test_assertion(a.as_integer() == 102, ("pass by shadow reference failed"));

    c = 18764;
    g1(c);
    test_assertion(c.as_integer() == 18764, ("pass by value failed"));
    test_assertion(perl.scalar("c").as_integer() == 18764, ("shadow variable isn't working"));

    g2(c);
    test_assertion(c.as_integer() == 102, ("pass by shadow reference failed"));
    test_assertion(perl.scalar("c").as_integer() == 102, ("shadow variable isn't working"));

    h1(a);
    test_assertion(a.as_integer() == 201, ("pass by &scalar failed"));

//    h2(a);	// compiler will warn about the use of a non-const temporary; the code
		// should work as expected, but is in poor style.

//    test_assertion(a.as_integer() == 202, ("pass by &shadow failed -- compiler should have warned"));

    h1(c);
    test_assertion(c.as_integer() == 201, ("pass by &scalar failed"));
    test_assertion(perl.scalar("c").as_integer() == 201, ("shadow variable isn't working"));

    h2(c);
    test_assertion(c.as_integer() == 202, ("pass by &shadow failed"));
    test_assertion(perl.scalar("c").as_integer() == 202, ("shadow variable isn't working"));

    wPerlScalar i = 0;
    wPerlScalar j;

    ++i;
    printf("i = %d\n", i.as_integer());
    test_assertion(i.as_integer() == 1, ("increment failed"));
    i++;
    printf("i = %d\n", i.as_integer());
    test_assertion(i.as_integer() == 2, ("increment failed"));
    j = i++;
    printf("i = %d\n", i.as_integer());
    test_assertion(i.as_integer() == 3, ("increment failed"));
    test_assertion(j.as_integer() == 2, ("assignment of postincrement failed"));
    j = ++i;
    test_assertion(i.as_integer() == 4, ("increment failed"));
    test_assertion(j.as_integer() == 4, ("assignment of preincrement failed"));
}

void test_overloaded_add()
{
    int i = 10 + 10;
    int j = (int)(10 + 20.0);
    double k = 10 + 30.0;
    wPerlScalar l = 10 + 40.0;
    int m = wPerlScalar(10).as_integer() + 50;
    wPerlScalar n = l + 20;
    wPerlScalar o = 30.0 + l;
    wPerlScalar ten = 10;
    wPerlScalar p = ten + ten + ten + ten + ten + ten + ten + ten + ten;
    wPerlScalar q = p + '1';

    printf("%d, %d, %f, %f, %d, %f, %f, %f\n", i, j, k, l.as_real(), m, n.as_real(), o.as_real(), p.as_real());

    test_assertion(i < j, ("overloaded add failed"));
    test_assertion(j < k, ("overloaded add failed"));
    test_assertion(k < l, ("overloaded add failed"));
    test_assertion(l < m, ("overloaded add failed"));
    test_assertion(m < n, ("overloaded add failed"));
    test_assertion(n < o, ("overloaded add failed"));
    test_assertion(o < p, ("overloaded add failed"));
}

void test_transmutation_of_scalars()
{
    wPerlScalar r = 0;
    wPerlScalar s = 0;

    for (int i = 0; i < 1000; ++i)
    {
	r += i;
    }

    test_assertion(r == 499500, ("r not computed correctly"));
    test_assertion(r.eq("499500"), ("r not transmuted to string correctly"));

    test_assertion(r, ("r as boolean failed"));
    test_assertion(!s, ("s as boolean failed"));
}

class Foo
{
    public:
	int i;

	Foo(int that_i) : i(that_i) { }
	Foo(const Foo &that) : i(17) { printf("copy Foo %d <- Foo %d\n", i, that.i); i = that.i; }
	~Foo() { printf("destroy Foo %d\n", i); }

	print() { printf("*** Foo %d\n", i); }
};

void test_array_templates()
{
    tPerlArray<Foo> a;
    Foo i = 1;
    Foo *p;

    i.print();

    a.set(0, i);
    p = a.get(0);

    p->print();
}

main()
{
    if (sizeof(wPerlScalar) > sizeof(void *))
    {
	printf("wPerlScalar probably won't be passed in a register.\n");
    }

    perl.use("lib", ".");
    if (!perl.use("example", "$X", "$Y", "%H"))
    {
	printf("couldn't load example.pm\n");
	exit(1);
    }

    wPerlScalar s = "This is\ta   test.";
    wPerlArray a;
    s.split(" +", &a);

    static char *good[] = { "This", "is\ta", "test." };
    for (int i = 0; i < a.length(); ++i)
    {
	printf("%s == %s\n", good[i], a.get(i).as_string());
	test_assertion(strcmp(good[i], a.get(i).as_string()) == 0, ("split didn't work"));
    }

    test_eval();
    test_binding();
    test_transmutation_of_scalars();
    test_replacing();
    test_searching();
    test_hashes();
    test_arithmetic();
    test_overloaded_add();
    test_arrays();
    test_array_templates();

    printf("%d tests out of %d passed.\n", tests_passed, tests_asserted);
    return tests_asserted - tests_passed;
}
