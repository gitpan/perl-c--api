
#include <EXTERN.h>
#include <perl.h>
#include <alloca.h>
#include <stdarg.h>

#undef ref
#undef scalar

#undef bool
#undef true
#undef false

#include "perl-glue.hh"

#define const_cast(TYPE, EXPR) ((TYPE)(EXPR))

extern "C" void xs_init(void);

wPerl *wPerl::running_interpreter = 0;

#define REQUIRE_PERL_RUNNING() \
    if (wPerl::running_interpreter == 0) new wPerl()

wPerl *wPerl::run()
{
    REQUIRE_PERL_RUNNING();
    return running_interpreter;
}

wPerl::wPerl()
{
    static char *argv[] = { "embedded_perl", "-e", 0, 0 };

    if ((perl = perl_alloc()) != 0)
    {
	int failure = 0;

	if (argv[2] == 0)
	{
	    static char STARTUP_BOILER_PLATE[] =
		"package ExtUtils::Embed::Glue;\n"

		"sub t_eval {"
		    "package main;"
		    "eval $_[0];"
		"}\n"

		"sub t_replace {"
		    "package main;"
		    "eval '$_[0] =~ s\001$_[1]\001'.$_[2].'\001g';"
		"}\n"

		"sub t_restart_search {"
		    "pos $_[0] = $_[1];"
		"}\n"

		"sub t_pos {"
		    "pos $_[0];"
		"}\n"

		"sub t_find {"
		    "$_[0] =~ m\001$_[1]\001;"
		"}\n"

		"sub t_find_next {"
		    "$_[0] =~ m\001$_[1]\001g;"
		"}\n"

		"sub t_bind_up_matches {"
		    "my $_i = 1; my $_x;"
		    "@{$_[0]} = ();"
		    "while ($_x = $main::{$_i}) {"
			"last if (!defined $$_x);"
			"push @{$_[0]}, $$_x;"
			"++$_i;"
		    "}"
		"}\n"

		"sub t_find_inquire {"
		    "my $_r = ($_[0] =~ m\001$_[1]\001);"
		    "ExtUtils::Embed::Glue::t_bind_up_matches($_[2]);"
		    "$_r;"
		"}\n"

		"sub t_find_inquire_next {"
		    "my $_r = ($_[0] =~ m\001$_[1]\001g);"
		    "ExtUtils::Embed::Glue::t_bind_up_matches($_[2]);"
		    "$_r;"
		"}\n"

		"sub t_find_all {"
		    "@{$_[2]} = ($_[0] =~ m\001$_[1]\001g);"
		    "scalar @{$_[2]};"
		"}\n"

		"sub t_find_all_count {"
		    "my @_r = ($_[0] =~ m\001$_[1]\001g);"
		    "scalar @_r;"
		"}\n"

		"sub t_compile_find {"
		    "package main;"
		    "eval '"
			"sub {"
			    "if ($_[0] =~ '.$_[0].') {"
				"ExtUtils::Embed::Glue::t_bind_up_matches($_[1]) if (defined $_[1]);"
				"1;"
			    "}"
			"}';"
		"}\n"

		"sub t_index {"
		    "index $_[0], $_[1], $_[2];"
		"}\n"

		"sub t_rindex {"
		    "if (defined $_[2]) {"
			"rindex $_[0], $_[1], $_[2];"
		    "} else {"
			"rindex $_[0], $_[1];"
		    "}"
		"}\n"

		"sub t_use {"
		    "package main;"
		    "my $_module = shift; my $_r;"
		    "$_r = eval 'require '.$_module;"
		    "if ($_r) {"
			"import $_module @_ if ($_r);"
		    "} else {"
			"print STDERR qq(error in use $_module: $@\\n);"
		    "}"
		    "$_r;"
		"}\n"

		"sub t_substr {"
		    "if (defined $_[2]) {"
			"substr $_[0], $_[1], $_[2];"
		    "} else {"
			"substr $_[0], $_[1];"
		    "}"
		"}\n"

		"sub t_rep_substr {"
		    "if (defined $_[3]) {"
			"substr($_[0], $_[1], $_[2]) = $_[3];"
		    "} else {"
			"substr($_[0], $_[1]) = $_[2];"
		    "}"
		"}\n"

		"sub t_split {"
		    "if (defined $_[3]) {"
			"@{$_[0]} = split($_[1], $_[2], $_[3]);"
		    "} else {"
			"@{$_[0]} = split($_[1], $_[2]);"
		    "}"
		"}\n"

		"sub t_sort_array {"
		    "@{$_[0]} = sort(@{$_[1]});"
		"}\n"

		"package main;\n"

		"use vars qw($1 $2 $3 $4 $5 $6 $7 $8 $9 $10 $11 $12 $13 $14 $15 $16 $17 $18 $19 $20);\n";

	    argv[2] = STARTUP_BOILER_PLATE;
	}

	perl_construct(perl);
	perl_parse(perl, xs_init, 3, argv, 0);
	perl_run(perl);

	/* Handles are kept to all the important variables and
	   functions so that they can be accessed without needing
	   a symbol table lookup. */

#define init(SUB) if ((SUB = (SV *)perl_get_cv("ExtUtils::Embed::Glue::" #SUB, FALSE)) == 0) { ++failure; }

	init(t_eval);
	init(t_replace);
	init(t_restart_search);
	init(t_pos);
	init(t_find);
	init(t_find_next);
	init(t_find_inquire);
	init(t_find_inquire_next);
	init(t_find_all);
	init(t_find_all_count);
	init(t_compile_find);
	init(t_index);
	init(t_rindex);
	init(t_use);
	init(t_substr);
	init(t_rep_substr);
	init(t_split);
	init(t_sort_array);

	use_locale = false;

#undef init

	/* If one of the variables or functions isn't found, that
	   usually means the module wasn't loaded properly.  This
	   means disaster, so just terminate perl. */

	if (failure > 0)
	{
	    perl_destruct(perl);
	    perl_free(perl);
	    perl = 0;
	}
    }

    if (perl && running_interpreter == 0)
    {
	running_interpreter = this;
    }
}

wPerl::~wPerl()
{
    if (running_interpreter != this)
    {
	if (perl != 0)
	{
	    perl_destruct(perl);
	    perl_free(perl);
	}
    }
    else
    {
	running_interpreter = 0;
    }

    perl = 0;
}

bool wPerl::internal_use(const char *module, int num, const char *functions[])
{
    bool status = false;

    if (perl != 0)
    {
	SV *result = 0;
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, module), strlen(module))));
	for (count = 0; count < num; ++count)
	{
	    XPUSHs(sv_2mortal(newSVpv(const_cast(char *, functions[count]), strlen(functions[count]))));
	}
	PUTBACK;

	count = perl_call_sv(t_use, G_SCALAR);

	SPAGAIN;

	if (count == 1)
	{
	    result = POPs;
	    status = SvTRUE(result);
	}

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return status;
}

SV *wPerl::internal_call(const SV *sub, int argc, const SV *argv[])
{
    SV *result = 0;

    if (perl != 0)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	for (count = 0; count < argc; ++count)
	{
	    XPUSHs(const_cast(SV *, argv[count]));
	}
	PUTBACK;

	count = perl_call_sv(const_cast(SV *, sub), G_SCALAR);

	SPAGAIN;

	if (count == 1)
	{
	    result = POPs;
	    result = (SvOK(result)) ? newSVsv(result) : 0;
	}

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return result;
}

#define A(N) const char *arg ## N

bool wPerl::use(const char *module)
{
    return internal_use(module, 0, 0);
}

bool wPerl::use(const char *module, A(1))
{
    const char *functions[1];
    functions[0] = arg1;
    return internal_use(module, 1, functions);
}

bool wPerl::use(const char *module, A(1), A(2))
{
    const char *functions[2];
    functions[0] = arg1;
    functions[1] = arg2;
    return internal_use(module, 2, functions);
}

bool wPerl::use(const char *module, A(1), A(2), A(3))
{
    const char *functions[3];
    functions[0] = arg1;
    functions[1] = arg2;
    functions[2] = arg3;
    return internal_use(module, 3, functions);
}

bool wPerl::use(const char *module, A(1), A(2), A(3), A(4))
{
    const char *functions[4];
    functions[0] = arg1;
    functions[1] = arg2;
    functions[2] = arg3;
    functions[3] = arg4;
    return internal_use(module, 4, functions);
}

#undef A

wPerlScalar wPerl::eval(const char *expr)
{
    SV *result = 0;

    if (perl != 0 && expr != 0)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, expr), strlen(expr))));
	PUTBACK;

	count = perl_call_sv(t_eval, G_SCALAR);

	SPAGAIN;

	if (count == 1) result = POPs;

	if (SvTRUE(GvSV(errgv)))
        {
	    STRLEN ignore;
	    fprintf(stderr, "eval '%s' failed: %s\n", expr, SvPV(GvSV(errgv), ignore));
	    result = 0;
	}
	else
	{
	    result = newSVsv(result);
	}

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return wPerlScalar(result, wPerlScalar::Keep);
}

wPerlScalarShadow wPerl::scalar(const char *name, LookupModifier should_create)
{
    SV *v = perl_get_sv(const_cast(char *, name), should_create == Create);
    return wPerlScalarShadow(v);
}

wPerlArrayShadow wPerl::array(const char *name, LookupModifier should_create)
{
    AV *v = perl_get_av(const_cast(char *, name), should_create == Create);
    return wPerlArrayShadow(v);
}

wPerlHashShadow wPerl::hash(const char *name, LookupModifier should_create)
{
    HV *v = perl_get_hv(const_cast(char *, name), should_create == Create);
    return wPerlHashShadow(v);
}

wPerlScalarShadow wPerl::subroutine(const char *name)
{
    CV *v = perl_get_cv(const_cast(char *, name), FALSE);
    if (v)
    {
	return wPerlScalarShadow(newRV_inc((SV *)v));
    }
    return wPerlScalarShadow();
}

void wPerlScalar::copy(const SV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (v)
	{
	    if (that_v)
	    {
		sv_setsv(v, const_cast(SV *, that_v));
	    }
	    else
	    {
		SvREFCNT_dec(v);
		v = 0;
	    }
	}
	else
	{
	    if (that_v)
	    {
		v = newSVsv(const_cast(SV *, that_v));
	    }
	}
    }
}

void wPerlScalar::share(SV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (v) SvREFCNT_dec(v);

	if (that_v)
	{
	    v = SvREFCNT_inc(that_v);
	}
	else
	{
	    v = 0;
	}
    }
}

SV *wPerlScalar::get_ref() const
{
    if (v) return v;
    return &sv_undef;
}

SV *wPerlScalar::get_shared_ref() const
{
    if (v)
    {
	return SvREFCNT_inc(v);
    }
    return &sv_undef;
}

bool wPerlScalar::defined() const
{
    return v != 0 && SvOK(v);
}

wPerlScalar::operator bool () const
{
    return v != 0 && SvTRUE(v);
}

wPerlScalar::wPerlScalar(const void *value, pPerlLength value_len) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_bytes(value, value_len);
}

wPerlScalar::wPerlScalar(const char *value) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_bytes(value, strlen(value));
}

wPerlScalar::wPerlScalar(int value) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_integer(value);
}

wPerlScalar::wPerlScalar(pPerlInteger value, Force_Integer_Type) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_integer(value);
}

wPerlScalar::wPerlScalar(double value) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_real(value);
}

wPerlScalar::wPerlScalar(pPerlReal value, Force_Real_Type) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_real(value);
}

wPerlScalar::wPerlScalar(void *value, Force_Pointer_Type) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_pointer(value);
}

wPerlScalar::wPerlScalar(void *value, const char *classname) : v(0)
{
    TR(Scalar)
    REQUIRE_PERL_RUNNING();
    set_as_integer(0);
    sv_setref_pv(v, const_cast(char *, classname), value);
}

void *wPerlScalar::set_as_bytes(const void *value, pPerlLength value_len)
{
    if (value == 0 && value_len > 0)
    {
	if (v)
	{
	    SvGROW(v, value_len);
	}
	else
	{
	    v = newSV(value_len);
	    SvPOK_on(v);
	}
    }
    else
    {
	if (value == 0)
	{
	    value = "";
	    value_len = 0;
	}

	if (v)
	{
	    sv_setpvn(v, const_cast(char *, value), value_len);
	}
	else
	{
	    v = newSVpv(const_cast(char *, value), value_len);
	}
    }
    return SvPVX(v);
}

void wPerlScalar::set_as_integer(pPerlInteger value)
{
    if (v)
    {
	sv_setiv(v, value);
    }
    else
    {
	v = newSViv(value);
    }
}

void wPerlScalar::set_as_pointer(void *value)
{
    if (v)
    {
	sv_setiv(v, (I32)value);
    }
    else
    {
	v = newSViv((I32)value);
    }
}

void wPerlScalar::set_as_real(pPerlReal value)
{
    if (v)
    {
	sv_setnv(v, value);
    }
    else
    {
	v = newSVnv(value);
    }
}

void wPerlScalar::add_additional(pPerlInteger value)
{
    if (v)
    {
	bool add_NV = SvNOK(v);
	bool add_PV = SvPOK(v);

	sv_setiv(v, value);

	if (add_NV) SvNOK_on(v);
	if (add_PV) SvPOK_on(v);
    }
}

wPerlScalar::~wPerlScalar()
{
    TR(~Scalar)
    if (v)
    {
	SvREFCNT_dec(v);
	v = 0;
    }
}

void *wPerlScalar::as_bytes(pPerlLength *len) const
{
    STRLEN n = 0;
    char *p = 0;

    if (v)
    {
	p = SvPV(v, n);
    }

    if (len)
    {
	*len = n;
    }
    return p;
}

char *wPerlScalar::as_string() const
{
    char *p = "";

    if (v)
    {
	STRLEN n;
	p = SvPV(v, n);
    }
    return p;
}

pPerlInteger wPerlScalar::as_integer() const
{
    if (v)
    {
	return SvIV(v);
    }
    return -1;
}

void *wPerlScalar::as_pointer() const
{
    if (v)
    {
	return (void *)SvIV(v);
    }
    return 0;
}

pPerlReal wPerlScalar::as_real() const
{
    if (v)
    {
	return SvNV(v);
    }
    return -1.0;
}

void *wPerlScalar::as_object(const char *classname) const
{
    if (v)
    {
	if (sv_derived_from(v, const_cast(char *, classname)))
	{
	    IV tmp = SvIV((SV *)SvRV(v));
	    return (void *)tmp;
	}
    }
    return 0;
}

wPerlScalarShadow wPerlScalar::deref_as_scalar()
{
    if (v)
    {
	SV *sv = SvRV(v);
	switch (SvTYPE(sv))
	{
	    case SVt_IV:
	    case SVt_NV:
	    case SVt_PV:
	    case SVt_RV:
	    case SVt_PVMG:
		return wPerlScalarShadow(sv);
	}
    }
    return wPerlScalarShadow();
}

wPerlArrayShadow wPerlScalar::deref_as_array()
{
    if (v)
    {
	AV *av = (AV *)SvRV(v);
	if (SvTYPE(av) == SVt_PVAV)
	{
	    return wPerlArrayShadow(av);
	}
    }
    return wPerlArrayShadow();
}

wPerlHashShadow wPerlScalar::deref_as_hash()
{
    if (v)
    {
	HV *hv = (HV *)SvRV(v);
	if (SvTYPE(hv) == SVt_PVHV)
	{
	    return wPerlHashShadow(hv);
	}
    }
    return wPerlHashShadow();
}

inline bool sv_is_true(SV *v)
{
    return v != 0 && SvTRUE(v);
}

bool wPerlScalar::replace(const char *pattern, const char *replacement) const
{
    bool found = false;

    if (v && wPerl::running_interpreter && pattern && replacement)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, replacement), strlen(replacement))));
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_replace, G_SCALAR);

	SPAGAIN;

	if (count == 1) found = sv_is_true(POPs);

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return found;
}

void wPerlScalar::restart_search(pPerlLength pos) const
{
    if (v && wPerl::running_interpreter)
    {
	dSP;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSViv(pos)));
	PUTBACK;

	perl_call_sv(wPerl::running_interpreter->t_restart_search, G_SCALAR|G_DISCARD);
    }
}

pPerlLength wPerlScalar::found_at() const
{
    pPerlLength pos = 0;

    if (v && wPerl::running_interpreter)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_pos, G_SCALAR);

	SPAGAIN;

	if (count == 1) pos = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return(pos);
}

pPerlLength wPerlScalar::length() const
{
    if (v)
    {
	return SvCUR(v);
    }
    return 0;
}

bool wPerlScalar::find(const char *pattern, wPerlArray *matches) const
{
    bool found = false;

    if (v && wPerl::running_interpreter && pattern)
    {
	SV *t_find;
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	if (matches)
	{
	    XPUSHs(sv_2mortal(matches->get_shared_ref()));
	    t_find = wPerl::running_interpreter->t_find_inquire;
	}
	else
	{
	    t_find = wPerl::running_interpreter->t_find;
	}
	PUTBACK;

	count = perl_call_sv(t_find, G_SCALAR);

	SPAGAIN;

	if (count == 1) found = sv_is_true(POPs);

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return found;
}

bool wPerlScalar::find_next(const char *pattern, wPerlArray *matches) const
{
    bool found = false;

    if (v && wPerl::running_interpreter && pattern)
    {
	SV *t_find_next;
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	if (matches)
	{
	    XPUSHs(sv_2mortal(matches->get_shared_ref()));
	    t_find_next = wPerl::running_interpreter->t_find_inquire_next;
	}
	else
	{
	    t_find_next = wPerl::running_interpreter->t_find_next;
	}
	PUTBACK;

	count = perl_call_sv(t_find_next, G_SCALAR);

	SPAGAIN;

	if (count == 1) found = sv_is_true(POPs);

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return found;
}

int wPerlScalar::find_all(const char *pattern, wPerlArray *matches) const
{
    int found = 0;

    if (v && wPerl::running_interpreter && pattern)
    {
	SV *t_find;
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	if (matches)
	{
	    XPUSHs(sv_2mortal(matches->get_shared_ref()));
	    t_find = wPerl::running_interpreter->t_find_all;
	}
	else
	{
	    t_find = wPerl::running_interpreter->t_find_all_count;
	}
	PUTBACK;

	count = perl_call_sv(t_find, G_SCALAR);

	SPAGAIN;

	if (count == 1) found = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return found;
}

bool wPerlScalar::apply_pattern(const wPerlPattern &pattern, wPerlArray *matches) const
{
    bool found = false;

    if (v && wPerl::running_interpreter && pattern.defined())
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	if (matches)
	{
	    XPUSHs(sv_2mortal(matches->get_shared_ref()));
	}
	PUTBACK;

	count = perl_call_sv(pattern.get_ref(), G_SCALAR);

	SPAGAIN;

	if (count == 1) found = sv_is_true(POPs);

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return found;
}

pPerlInteger wPerlScalar::index(const char *substr, pPerlLength offset) const
{
    pPerlInteger pos = -1;

    if (v && wPerl::running_interpreter && substr)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, substr), strlen(substr))));
	XPUSHs(sv_2mortal(newSViv(offset)));
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_index, G_SCALAR);

	SPAGAIN;

	if (count == 1) pos = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return pos;
}

pPerlInteger wPerlScalar::rindex(const char *substr, pPerlLength offset, bool useOffset) const
{
    pPerlInteger pos = -1;

    if (v && wPerl::running_interpreter && substr)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, substr), strlen(substr))));
	if (useOffset)
	{
	    XPUSHs(sv_2mortal(newSViv(offset)));
	}
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_rindex, G_SCALAR);

	SPAGAIN;

	if (count == 1) pos = POPi;

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return pos;
}

const wPerlScalar wPerlScalar::substr(pPerlLength offset, pPerlLength len, bool useLen)
{
    SV *result = 0;

    if (v && wPerl::running_interpreter)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSViv(offset)));
	if (useLen)
	{
	    XPUSHs(sv_2mortal(newSViv(len)));
	}
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_substr, G_SCALAR);

	SPAGAIN;

	if (count == 1)
	{
	    result = POPs;
	    result = (SvOK(result)) ? newSVsv(result) : 0;
	}

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    return wPerlScalar(result, wPerlScalar::Keep);
}

void wPerlScalar::replace_substr(const char *new_substr, pPerlLength offset, pPerlLength len, bool useLen)
{
    if (v && wPerl::running_interpreter && new_substr)
    {
	dSP;

	PUSHMARK(sp);
	XPUSHs(v);
	XPUSHs(sv_2mortal(newSViv(offset)));
	if (useLen)
	{
	    XPUSHs(sv_2mortal(newSViv(len)));
	}
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, new_substr), strlen(new_substr))));
	PUTBACK;

	perl_call_sv(wPerl::running_interpreter->t_rep_substr, G_SCALAR | G_DISCARD);
    }
}

void wPerlScalar::split(const char *pattern, wPerlArray *fields, pPerlIndex limit)
{
    if (v && wPerl::running_interpreter && pattern && fields)
    {
	dSP;

	fields->clear();

	PUSHMARK(sp);
	XPUSHs(sv_2mortal(fields->get_shared_ref()));
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	XPUSHs(v);
	if (limit > 0)
	{
	    XPUSHs(sv_2mortal(newSViv(limit)));
	}
	PUTBACK;

	perl_call_sv(wPerl::running_interpreter->t_split, G_SCALAR | G_DISCARD);
    }
}

wPerlScalar &wPerlScalar::operator += (int value)
{
    set_as_real(as_real() + value);
    return *this;
}

wPerlScalar &wPerlScalar::operator += (double value)
{
    set_as_real(as_real() + value);
    return *this;
}

wPerlScalar &wPerlScalar::operator += (const wPerlScalar &value)
{
    set_as_real(as_real() + value.as_real());
    return *this;
}

const wPerlScalar operator + (const wPerlScalar &x, const wPerlScalar &y)
{
    return wPerlScalar(x.as_real() + y.as_real());
}

const wPerlScalar wPerlScalar::operator ++ (int)
{
    wPerlScalar r = *this;
    set_as_real(as_real() + 1);
    return r;
}

wPerlScalar &wPerlScalar::operator ++ ()
{
    set_as_real(as_real() + 1);
    return *this;
}

wPerlScalar &wPerlScalar::operator -= (int value)
{
    set_as_real(as_real() - value);
    return *this;
}

wPerlScalar &wPerlScalar::operator -= (double value)
{
    set_as_real(as_real() - value);
    return *this;
}

wPerlScalar &wPerlScalar::operator -= (const wPerlScalar &value)
{
    set_as_real(as_real() - value.as_real());
    return *this;
}

const wPerlScalar operator - (const wPerlScalar &x, const wPerlScalar &y)
{
    return wPerlScalar(x.as_real() - y.as_real());
}

const wPerlScalar wPerlScalar::operator -- (int)
{
    wPerlScalar r = *this;
    set_as_real(as_real() - 1);
    return r;
}

wPerlScalar &wPerlScalar::operator -- ()
{
    set_as_real(as_real() - 1);
    return *this;
}

wPerlScalar &wPerlScalar::operator *= (int value)
{
    set_as_real(as_real() * value);
    return *this;
}

wPerlScalar &wPerlScalar::operator *= (double value)
{
    set_as_real(as_real() * value);
    return *this;
}

wPerlScalar &wPerlScalar::operator *= (const wPerlScalar &value)
{
    set_as_real(as_real() * value.as_real());
    return *this;
}

const wPerlScalar operator * (const wPerlScalar &x, const wPerlScalar &y)
{
    return wPerlScalar(x.as_real() * y.as_real());
}

wPerlScalar &wPerlScalar::operator /= (int value)
{
    set_as_real(as_real() / value);
    return *this;
}

wPerlScalar &wPerlScalar::operator /= (double value)
{
    set_as_real(as_real() / value);
    return *this;
}

wPerlScalar &wPerlScalar::operator /= (const wPerlScalar &value)
{
    set_as_real(as_real() / value.as_real());
    return *this;
}

const wPerlScalar operator / (const wPerlScalar &x, const wPerlScalar &y)
{
    return wPerlScalar(x.as_real() / y.as_real());
}

wPerlScalar &wPerlScalar::append(const void *value, pPerlLength value_len)
{
    if (value == 0)
    {
	value = "";
	value_len = 0;
    }

    if (v)
    {
	sv_catpvn(v, const_cast(char *, value), value_len);
    }
    else
    {
	v = newSVpv(const_cast(char *, value), value_len);
    }
    return *this;
}

wPerlScalar &wPerlScalar::append(const wPerlScalar &value)
{
    if (value.v)
    {
	if (v)
	{
	    sv_catsv(v, value.v);
	}
	else
	{
	    v = newSVsv(value.v);
	}
    }
    return *this;
}

#define cmp_string_scalars(A, B) \
    int cmp = (wPerl::running_interpreter->use_locale) \
		? sv_cmp_locale(A, B) \
		: sv_cmp(A, B)

bool wPerlScalar::lt(const wPerlScalar &string) const
{
    cmp_string_scalars(v, string.v);
    return cmp < 0;
}

bool wPerlScalar::le(const wPerlScalar &string) const
{
    cmp_string_scalars(v, string.v);
    return cmp <= 0;
}

bool wPerlScalar::ge(const wPerlScalar &string) const
{
    cmp_string_scalars(v, string.v);
    return cmp >= 0;
}

bool wPerlScalar::gt(const wPerlScalar &string) const
{
    cmp_string_scalars(v, string.v);
    return cmp > 0;
}

int wPerlScalar::cmp(const wPerlScalar &string) const
{
    cmp_string_scalars(v, string.v);
    return cmp;
}

#undef cmp_string_scalars

bool wPerlScalar::eq(const wPerlScalar &string) const
{
    return sv_cmp(v, string.v) == 0;
}

bool wPerlScalar::ne(const wPerlScalar &string) const
{
    return sv_cmp(v, string.v) != 0;
}

#define A(N) const wPerlScalar &arg ## N

wPerlScalar wPerlScalar::operator () ()
{
    if (v && wPerl::running_interpreter)
    {
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 0, 0), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[1];
	argv[0] = arg1.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 1, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[2];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 2, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[3];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 3, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3), A(4))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[4];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	argv[3] = arg4.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 4, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3), A(4), A(5))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[5];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	argv[3] = arg4.get_ref();
	argv[4] = arg5.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 5, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3), A(4), A(5), A(6))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[6];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	argv[3] = arg4.get_ref();
	argv[4] = arg5.get_ref();
	argv[5] = arg6.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 6, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3), A(4), A(5), A(6), A(7))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[7];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	argv[3] = arg4.get_ref();
	argv[4] = arg5.get_ref();
	argv[5] = arg6.get_ref();
	argv[6] = arg7.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 7, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

wPerlScalar wPerlScalar::operator () (A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8))
{
    if (v && wPerl::running_interpreter)
    {
	const SV *argv[8];
	argv[0] = arg1.get_ref();
	argv[1] = arg2.get_ref();
	argv[2] = arg3.get_ref();
	argv[3] = arg4.get_ref();
	argv[4] = arg5.get_ref();
	argv[5] = arg6.get_ref();
	argv[6] = arg7.get_ref();
	argv[7] = arg8.get_ref();
	return wPerlScalar(wPerl::running_interpreter->internal_call(v, 8, argv), wPerlScalar::Keep);
    }
    return wPerlScalar((SV *)0);
}

#undef A

bool operator < (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() < y.as_real();
}

bool operator <= (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() <= y.as_real();
}

bool operator >= (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() >= y.as_real();
}

bool operator > (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() > y.as_real();
}

bool operator == (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() == y.as_real();
}

bool operator != (const wPerlScalar &x, const wPerlScalar &y)
{
    return x.as_real() != y.as_real();
}

void wPerlPattern::compile(const char *pattern)
{
    if (v)
    {
	SvREFCNT_dec(v);
    }

    if (wPerl::running_interpreter && pattern)
    {
	int count;
	dSP;

	ENTER;
	SAVETMPS;

	PUSHMARK(sp);
	XPUSHs(sv_2mortal(newSVpv(const_cast(char *, pattern), strlen(pattern))));
	PUTBACK;

	count = perl_call_sv(wPerl::running_interpreter->t_compile_find, G_SCALAR);

	SPAGAIN;

	if (count == 1)
	{
	    v = POPs;
	    v = (SvTRUE(v)) ? newSVsv(v) : 0;
	}

	PUTBACK;

	FREETMPS;
	LEAVE;
    }
    else
    {
	v = 0;
    }
}

void wPerlArray::copy(const AV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (that_v)
	{
	    if (v)
	    {
		av_clear(v);
	    }
	    else
	    {
		v = newAV();
	    }

	    pPerlIndex len = AvFILL(that_v) + 1;
	    SV **entry;

	    av_extend(v, len);

	    for (pPerlIndex o = 0; o < len; ++o)
	    {
		entry = av_fetch(const_cast(AV *, that_v), o, FALSE);
		if (entry && *entry)
		{
		    av_store(v, o, newSVsv(*entry));
		}
	    }
	}
	else
	{
	    if (v)
	    {
		av_clear(v);
	    }
	}
    }
}

void wPerlArray::share(AV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (v) SvREFCNT_dec(v);

	if (that_v)
	{
	    v = (AV *)SvREFCNT_inc(that_v);
	}
	else
	{
	    v = 0;
	}
    }
}

bool wPerlArray::defined() const
{
    return v != 0;
}

wPerlArray::operator bool () const
{
    return v != 0 && AvFILL(v) + 1 > 0;
}

wPerlArray::wPerlArray()
{
    TR(Array)
    REQUIRE_PERL_RUNNING();
    v = newAV();
}

void wPerlArray::clear()
{
    if (v)
    {
	av_clear(v);
    }
    else
    {
	v = newAV();
    }
}

SV *wPerlArray::get_shared_ref() const
{
    if (v)
    {
	return newRV_inc((SV *)v);
    }
    return &sv_undef;
}

wPerlArray::~wPerlArray()
{
    TR(~Array)
    if (v)
    {
	SvREFCNT_dec(v);
	v = 0;
    }
}

pPerlIndex wPerlArray::length() const
{
    if (v)
    {
	return AvFILL(v) + 1;
    }
    return 0;
}

void wPerlArray::extend(pPerlIndex o)
{
    if (v && o > 0)
    {
	av_extend(v, o);
    }
}

bool wPerlArray::exists(pPerlIndex o) const
{
    if (v)
    {
	pPerlIndex len = AvFILL(v) + 1;

	if (o < 0)
	{
	    o += len;
	}
	return o >= 0 && o < len;
    }
    return false;
}

wPerlScalarShadow wPerlArray::get(pPerlIndex o) const
{
    SV **entry = 0;

    if (v)
    {
	entry = av_fetch(v, o, FALSE);
	if (entry)
	{
	    return wPerlScalarShadow(*entry);
	}
    }
    return wPerlScalarShadow();
}

wPerlScalar wPerlArray::pop()
{
    SV *entry = 0;

    if (v)
    {
	entry = av_pop(v);
    }
    return wPerlScalar(entry);
}

wPerlScalar wPerlArray::shift()
{
    SV *entry = 0;

    if (v)
    {
	entry = av_shift(v);
    }
    return wPerlScalar(entry);
}

wPerlArray &wPerlArray::operator >> (wPerlScalar &value)
{
    if (v)
    {
	value.copy(av_shift(v));
    }
    return *this;
}

void wPerlArray::set_shared(pPerlIndex o, const wPerlScalar &value)
{
    if (v)
    {
	av_store(v, o, value.get_shared_ref());
    }
}

void wPerlArray::set_as_pointer(pPerlIndex o, void *value)
{
    if (v)
    {
	av_store(v, o, newSViv((I32)value));
    }
}

pPerlIndex wPerlArray::push_shared(const wPerlScalar &value)
{
    if (v)
    {
	av_push(v, value.get_shared_ref());
	return AvFILL(v);
    }
    return -1;
}

wPerlArray &wPerlArray::operator << (wPerlScalar value)
{
    if (v)
    {
	av_push(v, value.get_shared_ref());
    }
    return *this;
}

void wPerlArray::unshift_shared(const wPerlScalar &value)
{
    if (v)
    {
	av_unshift(v, 1);
	av_store(v, 0, value.get_shared_ref());
    }
}

wPerlArrayShadow wPerlArray::sort()
{
    wPerlArray sorted;

    if (v && wPerl::running_interpreter)
    {
	dSP;

	PUSHMARK(sp);
	XPUSHs(sv_2mortal(sorted.get_shared_ref()));
	XPUSHs(sv_2mortal(get_shared_ref()));
	PUTBACK;

	perl_call_sv(wPerl::running_interpreter->t_sort_array, G_SCALAR | G_DISCARD);
    }
    return sorted;
}

wPerlArray &wPerlArray::append(const wPerlArray &that)
{
    if (that.v)
    {
	pPerlIndex that_len = AvFILL(that.v) + 1;

	if (that_len > 0)
	{
	    pPerlIndex dst = 0;

	    if (v)
	    {
		dst = AvFILL(v) + 1;
	    }
	    else
	    {
		v = newAV();
	    }

	    av_extend(v, that_len + dst);

	    SV **entry;

	    for (pPerlIndex src = 0; src < that_len; ++src, ++dst)
	    {
		entry = av_fetch(const_cast(AV *, that.v), src, FALSE);
		if (entry && *entry)
		{
		    av_store(v, dst, newSVsv(*entry));
		}
	    }
	}
    }
    return *this;
}

void *wPerlArray::get_pv(pPerlIndex o) const
{
    if (v)
    {
	SV **entry = av_fetch(v, o, FALSE);

	if (entry)
	{
	    STRLEN len;
	    return SvPV(*entry, len);
	}
    }
    return 0;
}

pPerlInteger wPerlArray::get_iv(pPerlIndex o) const
{
    if (v)
    {
	SV **entry = av_fetch(v, o, FALSE);

	if (entry)
	{
	    return SvIV(*entry);
	}
    }
    return -1;
}

void *wPerlArray::get_iv_as_pointer(pPerlIndex o) const
{
    if (v)
    {
	SV **entry = av_fetch(v, o, FALSE);

	if (entry)
	{
	    return (void *)SvIV(*entry);
	}
    }
    return 0;
}

pPerlReal wPerlArray::get_nv(pPerlIndex o) const
{
    if (v)
    {
	SV **entry = av_fetch(v, o, FALSE);

	if (entry)
	{
	    return SvNV(*entry);
	}
    }
    return -1.0;
}

void wPerlHash::copy(const HV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (that_v)
	{
	    if (v)
	    {
		hv_clear(v);
	    }
	    else
	    {
		v = newHV();
	    }

	    hv_iterinit(const_cast(HV *, that_v));

	    HE *he;
	    I32 key_len;
	    char *key;
	    SV *entry;

	    while ((he = hv_iternext(const_cast(HV *, that_v))) != 0)
	    {
		key = hv_iterkey(he, &key_len);
		entry = hv_iterval(const_cast(HV *, that_v), he);
		hv_store(v, key, key_len, (entry) ? newSVsv(entry) : &sv_undef, HeHASH(he));
	    }
	}
	else
	{
	    if (v)
	    {
		hv_clear(v);
	    }
	}
    }
}

void wPerlHash::share(HV *that_v)
{
    REQUIRE_PERL_RUNNING();

    if (v != that_v)
    {
	if (v) SvREFCNT_dec(v);

	if (that_v)
	{
	    v = (HV *)SvREFCNT_inc(that_v);
	}
	else
	{
	    v = 0;
	}
    }
}

bool wPerlHash::defined() const
{
    return v != 0;
}

wPerlHash::operator bool () const
{
    // This is not complete because it doesn't work (correctly) for magic hashes.  FIXME
    return v != 0 && HvKEYS(v) > 0;
}

wPerlHash::wPerlHash()
{
    TR(Hash)
    REQUIRE_PERL_RUNNING();
    v = newHV();
}

void wPerlHash::clear()
{
    if (v)
    {
	hv_clear(v);
    }
    else
    {
	v = newHV();
    }
}

SV *wPerlHash::get_shared_ref() const
{
    if (v)
    {
	return newRV_inc((SV *)v);
    }
    return &sv_undef;
}

wPerlHash::~wPerlHash()
{
    TR(~Hash)
    if (v)
    {
	SvREFCNT_dec(v);
	v = 0;
    }
}

bool wPerlHash::exists(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	return hv_exists(v, const_cast(char *, key), key_len);
    }
    return false;
}

wPerlScalarShadow wPerlHash::get(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	SV **entry = hv_fetch(v, const_cast(char *, key), key_len, FALSE);

	if (entry)
	{
	    return wPerlScalarShadow(*entry);
	}
    }
    return wPerlScalarShadow();
}

void *wPerlHash::get_pv(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	SV **entry = hv_fetch(v, const_cast(char *, key), key_len, FALSE);

	if (entry)
	{
	    STRLEN len;
	    return SvPV(*entry, len);
	}
    }
    return 0;
}

pPerlInteger wPerlHash::get_iv(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	SV **entry = hv_fetch(v, const_cast(char *, key), key_len, FALSE);

	if (entry)
	{
	    return SvIV(*entry);
	}
    }
    return -1;
}

void *wPerlHash::get_iv_as_pointer(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	SV **entry = hv_fetch(v, const_cast(char *, key), key_len, FALSE);

	if (entry)
	{
	    return (void *)SvIV(*entry);
	}
    }
    return 0;
}

pPerlReal wPerlHash::get_nv(const void *key, pPerlLength key_len) const
{
    if (v)
    {
	SV **entry = hv_fetch(v, const_cast(char *, key), key_len, FALSE);

	if (entry)
	{
	    return SvNV(*entry);
	}
    }
    return -1.0;
}

void wPerlHash::remove(const void *key, pPerlLength key_len)
{
    if (v)
    {
	hv_delete(v, const_cast(char *, key), key_len, G_DISCARD);
    }
}

void wPerlHash::set_shared(const void *key, pPerlLength key_len, const wPerlScalar &value)
{
    if (v)
    {
	bool magical = SvMAGICAL(v);
	SV **entry = hv_store(v, const_cast(char *, key), key_len, value.get_shared_ref(), 0);

	if (entry && magical)
	{
	    mg_set(*entry);
	}
    }
}

void wPerlHash::set_as_pointer(const void *key, pPerlLength key_len, void *value)
{
    if (v)
    {
	bool magical = SvMAGICAL(v);
	SV **entry = hv_store(v, const_cast(char *, key), key_len, newSViv((I32)value), 0);

	if (entry && magical)
	{
	    mg_set(*entry);
	}
    }
}

bool wPerlHash::each(char **key, wPerlScalar *value) const
{
    if (v)
    {
	HE *he = hv_iternext(v);

	if (he)
	{
	    I32 retlen;

	    *key = hv_iterkey(he, &retlen);
	    if (value)
	    {
		value->share(hv_iterval(v, he));
	    }
	    return true;
	}
    }
    return false;
}

void *wPerlHash::each(pPerlLength *key_len, wPerlScalar *value) const
{
    char *key = 0;

    if (v)
    {
	HE *he = hv_iternext(v);

	if (he)
	{
	    I32 retlen;

	    key = hv_iterkey(he, &retlen);
	    *key_len = retlen;
	    if (value)
	    {
		value->share(hv_iterval(v, he));
	    }
	}
    }
    return key;
}

void wPerlHash::reset() const
{
    if (v)
    {
	hv_iterinit(v);
    }
}

wPerlArrayShadow wPerlHash::keys() const
{
    wPerlArray keys;

    if (v)
    {
	hv_iterinit(const_cast(HV *, v));

	HE *he;
	I32 key_len;
	char *key;

	while ((he = hv_iternext(const_cast(HV *, v))) != 0)
	{
	    key = hv_iterkey(he, &key_len);
	    keys << wPerlScalar(key, key_len);
	}
    }
    return keys;
}
