
#ifndef _PERL_GLUE_HH
#define _PERL_GLUE_HH

#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7))
# define HAS_BOOL
# define HAS_TEMPLATES
#endif

#if defined(__SUNPRO_CC)
# define USE_INT_BOOL
# define HAS_TEMPLATES
#endif

#ifndef HAS_BOOL
# ifdef USE_INT_BOOL
    typedef int bool;
    const bool false = 0;
    const bool true = 1;
# else
    typedef enum { false = 0, true = 1 } bool;
# endif
#endif

#include <string.h>

typedef struct sv SV;
typedef struct av AV;
typedef struct hv HV;
typedef struct interpreter PerlInterpreter;

class wPerlPattern;
class wPerlScalarShadow;
class wPerlArrayShadow;
class wPerlHashShadow;

// Naming conventions:
//
// wPerl_ ... macro used for wrappers
// wPerl  ... wrapped Perl type
// pPerl  ... primitive Perl type
// tPerl  ... template Perl type
//
// A "Perl" namespace will eventually be added but the current
// names will be #define'd for compatibility.

/* The primitive types need to be read from the local Perl configuration.
   Hopefully the values for C++ will be the same as those for C. */

typedef double pPerlReal;		// real (double)
typedef int pPerlInteger;		// integer (I32)
typedef int pPerlIndex;			// array index (I32)
typedef unsigned int pPerlLength;	// scalar length (STRLEN)


/* I'd rather use templates than these weird macros, but I haven't figured
   out how.  The trouble is that the wrapper classes have some primitive
   behavior that can be specified with a template and other behavior unique to
   each wrapper.  I thought the specific wrappers could inherit from a
   template wrapper, but I couldn't make the constructors work right.  Anybody
   have a solution? */

//#define DEBUG_PERL_CONSTRUCTION

#ifdef DEBUG_PERL_CONSTRUCTION
# define TR(N) printf("0x%08x::" #N "\n", (unsigned int)this);
# define TRs(N) printf("0x%08x::" #N "Shadow\n", (unsigned int)this);
#else
# define TR(N)
# define TRs(N)
#endif

#ifndef USE_INT_BOOL
# define BOOL_CAST operator bool () const { return is_true(); }
#else
# define BOOL_CAST
#endif

#define wPerl_ValueInterface(N, T) \
    friend class wPerl;											\
    friend class wPerl ## N ## Shadow;									\
    public:												\
	enum XV_Share { Share };									\
	enum XV_Keep { Keep };										\
    public:												\
	T *v;												\
    public:												\
	void copy(const T *that_v);									\
	void share(T *that_v);										\
	const wPerl ## N *share() const									\
	    { return new wPerl ## N(v, Share); }							\
	wPerl ## N *share()										\
	    { return new wPerl ## N(v, Share); }							\
	SV *get_shared_ref() const;									\
	wPerl ## N(const T *that_v) : v(0)								\
	    { TR(N) copy(that_v); }									\
	wPerl ## N(T *that_v, XV_Share) : v(0)								\
	    { TR(N) share(that_v); }									\
	wPerl ## N(T *that_v, XV_Keep) : v(that_v)							\
	    { TR(N) }											\
    public:												\
	~wPerl ## N();											\
	bool defined() const;										\
	bool is_true() const;										\
	BOOL_CAST											\
	wPerl ## N(const wPerl ## N &that) : v(0)							\
	    { TR(N) copy(that.v); }									\
	wPerl ## N &operator = (const wPerl ## N &that)							\
	    { copy(that.v); return *this; }								\
	wPerl ## N &operator = (const wPerl ## N ## Shadow &that);					\
	void rebind(const wPerl ## N &that)								\
	    { share(that.v); }

#define wPerl_ShadowInterface(N, T) \
    friend class wPerl;											\
    friend class wPerl ## N;										\
    public:												\
	wPerl ## N ## Shadow(T *that_v = 0) : wPerl ## N(that_v, wPerl ## N::Share)			\
	    { TRs(N) }											\
	wPerl ## N ## Shadow(T *that_v, wPerl ## N::XV_Keep) : wPerl ## N(that_v, wPerl ## N::Keep)	\
	    { TRs(N) }											\
    public:												\
	wPerl ## N ## Shadow(const wPerl ## N ## Shadow &that) : wPerl ## N(that.v, wPerl ## N::Share)	\
	    { TRs(N) }											\
	wPerl ## N ## Shadow &operator = (const wPerl ## N ## Shadow &that)				\
	    { copy(that.v); return *this; }								\
	wPerl ## N ## Shadow(const wPerl ## N &that);							\
	wPerl ## N ## Shadow &operator = (const wPerl ## N &that);

#define wPerl_CTOR(C) inline C::C
#define wPerl_ASSN(C) inline C &C::operator =

#define wPerl_ShadowConversions(N, T) \
    wPerl_ASSN(wPerl ## N)(const wPerl ## N ## Shadow &that)						\
	{ copy(that.v); return *this; }									\
    wPerl_CTOR(wPerl ## N ## Shadow)(const wPerl ## N &that) : wPerl ## N(that.v, wPerl ## N::Share)	\
	{ TRs(N) }											\
    wPerl_ASSN(wPerl ## N ## Shadow)(const wPerl ## N &that)						\
	{ copy(that.v); return *this; }


class wPerlScalar
{
    friend class wPerlArray;
    friend class wPerlHash;

    wPerl_ValueInterface(Scalar, SV)

    public:
	enum Force_Integer_Type { Force_Integer };
	enum Force_Real_Type { Force_Real };
	enum Force_Pointer_Type { Force_Pointer };

    protected:
	SV *get_ref() const;

    public:
	wPerlScalar() : v(0)
	    { TR(Scalar) }

	void clobber_closure();

	wPerlScalar(const void *value, pPerlLength value_len);
	wPerlScalar(const char *value);
	wPerlScalar(int value);
	wPerlScalar(pPerlInteger value, Force_Integer_Type);
	wPerlScalar(double value);
	wPerlScalar(pPerlReal value, Force_Real_Type);
	wPerlScalar(void *value, Force_Pointer_Type);
	wPerlScalar(void *value, const char *classname);

	wPerlScalar &operator = (const char *value)
	    { set_as_bytes(value, strlen(value)); return *this; }
	wPerlScalar &operator = (int value)
	    { set_as_integer(value); return *this; }
	wPerlScalar &operator = (double value)
	    { set_as_real(value); return *this; }

	void *set_as_bytes(const void *value, pPerlLength value_len);
	void set_as_string(const char *value)
	    { set_as_bytes(value, strlen(value)); }
	void set_as_integer(pPerlInteger value);
	void set_as_pointer(void *value);
	void set_as_real(pPerlReal value);

	// this is fragile.  any way to get it to preserve all the
	// types even when one is modified?  FIXME
	void add_additional(pPerlInteger value);

	void *as_bytes(pPerlLength *len = 0) const;
	char *as_string() const;
	pPerlInteger as_integer() const;
	void *as_pointer() const;
	pPerlReal as_real() const;
	void *as_object(const char *classname) const;

	wPerlScalarShadow deref_as_scalar();
	wPerlArrayShadow deref_as_array();
	wPerlHashShadow deref_as_hash();

	bool isa_subroutine() const;
	bool isa_ref() const;
	bool isa_scalar_ref() const;
	bool isa_array_ref() const;
	bool isa_hash_ref() const;

	bool replace(const char *pattern, const char *replacement) const;

	void restart_search(pPerlLength start_at = 0) const;
	pPerlLength found_at() const;

	bool find(const char *pattern, wPerlArray *matches = 0) const;
	bool find_next(const char *pattern, wPerlArray *matches = 0) const;
	int find_all(const char *pattern, wPerlArray *matches = 0) const;
	bool apply_pattern(const wPerlPattern &pattern, wPerlArray *matches = 0) const;

	pPerlLength length() const;

	pPerlInteger index(const char *substr, pPerlLength offset = 0) const;
	pPerlInteger rindex(const char *substr, pPerlLength offset, bool useOffset = true) const;
	pPerlInteger rindex(const char *substr) const
	    { return rindex(substr, 0, false); }

	const wPerlScalar substr(pPerlLength offset, pPerlLength len, bool useLen = true);
	const wPerlScalar substr(pPerlLength offset)
	    { return substr(offset, 0, false); }
	void replace_substr(const char *new_substr, pPerlLength offset, pPerlLength len, bool useLen = true);
	void replace_substr(const char *new_substr, pPerlLength offset)
	    { replace_substr(new_substr, offset, 0, false); }

	void split(const char *pattern, wPerlArray *fields, pPerlIndex limit = 0);

	wPerlScalar &operator += (int value);
	wPerlScalar &operator += (double value);
	wPerlScalar &operator += (const wPerlScalar &value);

	const wPerlScalar operator ++ (int);	// postfix
	wPerlScalar &operator ++ ();		// prefix

	wPerlScalar &operator -= (int value);
	wPerlScalar &operator -= (double value);
	wPerlScalar &operator -= (const wPerlScalar &value);

	const wPerlScalar operator -- (int);	// postfix
	wPerlScalar &operator -- ();		// prefix

	wPerlScalar &operator *= (int value);
	wPerlScalar &operator *= (double value);
	wPerlScalar &operator *= (const wPerlScalar &value);

	wPerlScalar &operator /= (int value);
	wPerlScalar &operator /= (double value);
	wPerlScalar &operator /= (const wPerlScalar &value);

	wPerlScalar &append(const void *value, pPerlLength value_len);
	wPerlScalar &append(const char *value)
	    { return append(value, strlen(value)); }
	wPerlScalar &append(const wPerlScalar &value);

	wPerlScalar &prepend(const void *value, pPerlLength value_len);
	wPerlScalar &prepend(const char *value)
	    { return prepend(value, strlen(value)); }
	wPerlScalar &prepend(const wPerlScalar &value);

	bool lt(const wPerlScalar &string) const;
	bool le(const wPerlScalar &string) const;
	bool ge(const wPerlScalar &string) const;
	bool gt(const wPerlScalar &string) const;
	int cmp(const wPerlScalar &string) const;

	bool eq(const wPerlScalar &string) const;
	bool ne(const wPerlScalar &string) const;

#define A(N) const wPerlScalar &arg ## N
	wPerlScalar operator () () const;
	wPerlScalar operator () (A(1)) const;
	wPerlScalar operator () (A(1), A(2)) const;
	wPerlScalar operator () (A(1), A(2), A(3)) const;
	wPerlScalar operator () (A(1), A(2), A(3), A(4)) const;
	wPerlScalar operator () (A(1), A(2), A(3), A(4), A(5)) const;
	wPerlScalar operator () (A(1), A(2), A(3), A(4), A(5), A(6)) const;
	wPerlScalar operator () (A(1), A(2), A(3), A(4), A(5), A(6), A(7)) const;
	wPerlScalar operator () (A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8)) const;
#undef A
};

const wPerlScalar operator + (const wPerlScalar &x, const wPerlScalar &y);
const wPerlScalar operator - (const wPerlScalar &x, const wPerlScalar &y);
const wPerlScalar operator * (const wPerlScalar &x, const wPerlScalar &y);
const wPerlScalar operator / (const wPerlScalar &x, const wPerlScalar &y);

bool operator <  (const wPerlScalar &x, const wPerlScalar &y);
bool operator <= (const wPerlScalar &x, const wPerlScalar &y);
bool operator >= (const wPerlScalar &x, const wPerlScalar &y);
bool operator >  (const wPerlScalar &x, const wPerlScalar &y);

bool operator == (const wPerlScalar &x, const wPerlScalar &y);
bool operator != (const wPerlScalar &x, const wPerlScalar &y);

class wPerlScalarShadow: public wPerlScalar
{
    friend class wPerlArray;
    friend class wPerlHash;

    wPerl_ShadowInterface(Scalar, SV)

    public:
	wPerlScalarShadow &operator = (const char *value)
	    { set_as_bytes(value, strlen(value)); return *this; }
	wPerlScalarShadow &operator = (int value)
	    { set_as_integer(value); return *this; }
	wPerlScalarShadow &operator = (double value)
	    { set_as_real(value); return *this; }
};

wPerl_ShadowConversions(Scalar, SV)

class wPerlPattern: protected wPerlScalar
{
    public:
	wPerlPattern(const wPerlPattern &that) : wPerlScalar(that.v)
	    { }
	wPerlPattern(const char *pattern = 0) : wPerlScalar(0, wPerlScalar::Keep)
	    { compile(pattern); }

	~wPerlPattern()
	    { clobber_closure(); }

	wPerlPattern &operator = (const wPerlPattern &that)
	    { copy(that.v); return *this; }
	wPerlPattern &operator = (const char *pattern)
	    { compile(pattern); return *this; }

	void compile(const char *pattern);

	bool defined() const
	    { return wPerlScalar::defined(); }
};


class wPerlArray
{
    friend class wPerlScalar;

    wPerl_ValueInterface(Array, AV)

    public:
	wPerlArray();

	void clear();

	pPerlIndex length() const;
	void extend(pPerlIndex o);

	bool exists(pPerlIndex o) const;

	wPerlScalarShadow get(pPerlIndex o) const;
	wPerlScalarShadow operator [] (pPerlIndex o) const
	    { return get(o); }
	wPerlScalar pop();
	wPerlScalar shift();
	wPerlArray &operator >> (wPerlScalar &value);

	void set_shared(pPerlIndex o, const wPerlScalar &value);
	pPerlIndex push_shared(const wPerlScalar &value);
	void unshift_shared(const wPerlScalar &value);

	void set(pPerlIndex o, wPerlScalar value)
	    { set_shared(o, value); }
	pPerlIndex push(wPerlScalar value)
	    { return push_shared(value); }
	wPerlArray &operator << (wPerlScalar value);
	void unshift(wPerlScalar value)
	    { unshift_shared(value); }

	void remove(pPerlIndex o, pPerlLength count = 1);

	wPerlArrayShadow sort();

	wPerlArray &append(const wPerlArray &that);

	void set_as_bytes(pPerlIndex o, const void *value, pPerlLength value_len)
	    { set_shared(o, wPerlScalar(value, value_len)); }
	void set_as_string(pPerlIndex o, const char *value)
	    { set_shared(o, wPerlScalar(value, strlen(value))); }
	void set_as_integer(pPerlIndex o, pPerlInteger value)
	    { set_shared(o, wPerlScalar(value, wPerlScalar::Force_Integer)); }
	void set_as_pointer(pPerlIndex o, void *value);
	void set_as_real(pPerlIndex o, pPerlReal value)
	    { set_shared(o, wPerlScalar(value, wPerlScalar::Force_Real)); }

	pPerlIndex push_as_bytes(const void *value, pPerlLength value_len)
	    { return push_shared(wPerlScalar(value, value_len)); }
	pPerlIndex push_as_string(const char *value)
	    { return push_shared(wPerlScalar(value)); }
	pPerlIndex push_as_integer(pPerlInteger value)
	    { return push_shared(wPerlScalar(value, wPerlScalar::Force_Integer)); }
	pPerlIndex push_as_real(pPerlReal value)
	    { return push_shared(wPerlScalar(value, wPerlScalar::Force_Real)); }

	void unshift_as_bytes(const void *value, pPerlLength value_len)
	    { unshift_shared(wPerlScalar(value, value_len)); }
	void unshift_as_string(const char *value)
	    { unshift_shared(wPerlScalar(value)); }
	void unshift_as_integer(pPerlInteger value)
	    { unshift_shared(wPerlScalar(value, wPerlScalar::Force_Integer)); }
	void unshift_as_real(pPerlReal value)
	    { unshift_shared(wPerlScalar(value, wPerlScalar::Force_Real)); }

	void *get_pv(pPerlIndex o) const;
	pPerlInteger get_iv(pPerlIndex o) const;
	void *get_iv_as_pointer(pPerlIndex o) const;
	pPerlReal get_nv(pPerlIndex o) const;

	operator wPerlScalar () const
	    { return wPerlScalar(get_shared_ref(), wPerlScalar::Keep); }
};

class wPerlArrayShadow: public wPerlArray
{
    friend class wPerlScalar;

    wPerl_ShadowInterface(Array, AV)
};

wPerl_ShadowConversions(Array, AV)

#ifdef HAS_TEMPLATES

#include <new.h>

template <class T> class tPerlArray
{
    protected:
	wPerlArray array;

    public:
	T *get(pPerlIndex o) const
	    { return (T *)array.get_pv(o); }

	T *operator [] (pPerlIndex o) const
	    { return (T *)array.get_pv(o); }

	T *set(pPerlIndex o, const T &value)
	    {
		T *p = (T *)array.get_pv(o);

		if (p)
		{
		    *p = value;
		}
		else
		{
		    wPerlScalar s(0, sizeof(T));
		    array.set_shared(o, s);
		    p = new (s.as_bytes()) T(value);
		}

		return p;
	    }

	void remove(pPerlIndex o, pPerlLength count = 1)
	    {
		if (o < 0) o += array.length();
		if (o < 0) o = 0;
		T *p;
		pPerlLength i = 0;
		while (i < count)
		{
		    p = (T *)array.get_pv(o + i);
		    if (p) p->T::~T();
		    ++i;
		}
		array.remove(o, count);
	    }

	pPerlIndex push(const T &value)
	    {
		wPerlScalar s(0, sizeof(T));
		pPerlIndex o = array.push_shared(s);
		new (s.as_bytes()) T(value);
		return o;
	    }

	void clear()
	    { array.clear(); }

	pPerlIndex length() const
	    { return array.length(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)array; }
};

template <class T> class tPerlArrayI
{
    protected:
	wPerlArray array;

    public:
	T get(pPerlIndex o) const
	    { return (T)array.get_iv(o); }

	void set(pPerlIndex o, T value)
	    { array.set_as_integer(o, value); }

	pPerlIndex length() const
	    { return array.length(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)array; }
};

template <class T> class tPerlArrayP
{
    protected:
	wPerlArray array;

    public:
	T get(pPerlIndex o) const
	    { return (T)array.get_iv_as_pointer(o); }

	void set(pPerlIndex o, T value)
	    { array.set_as_pointer(o, value); }

	void extend(pPerlIndex o)
	    { array.extend(o); }

	pPerlIndex length() const
	    { return array.length(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)array; }
};

#endif /* HAS_TEMPLATES */


class wPerlHash
{
    friend class wPerlScalar;

    wPerl_ValueInterface(Hash, HV)

    public:
	wPerlHash();

	void clear();

	bool exists(const void *key, pPerlLength key_len) const;
	bool exists(const char *key) const
	    { return exists(key, strlen(key)); }
	bool exists(int key) const
	    { return exists(&key, sizeof(int)); }

	wPerlScalarShadow get(const void *key, pPerlLength key_len) const;
	wPerlScalarShadow get(const char *key) const
	    { return get(key, strlen(key)); }
	wPerlScalarShadow get(int key) const
	    { return get(&key, sizeof(int)); }

	wPerlScalarShadow operator [] (int key) const
	    { return get(&key, sizeof(int)); }
	wPerlScalarShadow operator [] (const char *key) const
	    { return get(key, strlen(key)); }

	void remove(const void *key, pPerlLength key_len);
	void remove(const char *key)
	    { remove(key, strlen(key)); }
	void remove(int key)
	    { remove(&key, sizeof(int)); }

	void set_shared(const void *key, pPerlLength key_len, const wPerlScalar &value);

	void set(const void *key, pPerlLength key_len, wPerlScalar value)
	    { set_shared(key, key_len, value); }
	void set(const char *key, wPerlScalar value)
	    { set_shared(key, strlen(key), value); }
	void set(int key, wPerlScalar value)
	    { set_shared(&key, sizeof(int), value); }

	void set_shared(const char *key, const wPerlScalar &value)
	    { set_shared(key, strlen(key), value); }
	void set_shared(int key, const wPerlScalar &value)
	    { set_shared(&key, sizeof(int), value); }

	void *set_as_bytes(const void *key, pPerlLength key_len, const void *value, pPerlLength value_len)
	    {
		wPerlScalar s(value, value_len);
		set_shared(key, key_len, s);
		return s.as_bytes();
	    }

	void *set_as_bytes(const char *key, const void *value, pPerlLength value_len)
	    { return set_as_bytes(key, strlen(key), value, value_len); }
	void *set_as_bytes(int key, const void *value, pPerlLength value_len)
	    { return set_as_bytes(&key, sizeof(int), value, value_len); }

	void set_as_string(const void *key, pPerlLength key_len, const char *value)
	    { set_shared(key, key_len, wPerlScalar(value, strlen(value))); }
	void set_as_string(const char *key, const char *value)
	    { set_shared(key, strlen(key), wPerlScalar(value, strlen(value))); }
	void set_as_string(int key, const char *value)
	    { set_shared(&key, sizeof(int), wPerlScalar(value, strlen(value))); }

	void set_as_integer(const void *key, pPerlLength key_len, pPerlInteger value)
	    { set_shared(key, key_len, wPerlScalar(value, wPerlScalar::Force_Integer)); }
	void set_as_integer(const char *key, pPerlInteger value)
	    { set_shared(key, strlen(key), wPerlScalar(value, wPerlScalar::Force_Integer)); }
	void set_as_integer(int key, pPerlInteger value)
	    { set_shared(&key, sizeof(int), wPerlScalar(value, wPerlScalar::Force_Integer)); }

	void set_as_pointer(const void *key, pPerlLength key_len, void *value);
	void set_as_pointer(const char *key, void *value)
	    { set_as_pointer(key, strlen(key), value); }
	void set_as_pointer(int key, void *value)
	    { set_as_pointer(&key, sizeof(int), value); }

	void set_as_real(const void *key, pPerlLength key_len, pPerlReal value)
	    { set_shared(key, key_len, wPerlScalar(value, wPerlScalar::Force_Real)); }
	void set_as_real(const char *key, pPerlReal value)
	    { set_shared(key, strlen(key), wPerlScalar(value, wPerlScalar::Force_Real)); }
	void set_as_real(int key, pPerlReal value)
	    { set_shared(&key, sizeof(int), wPerlScalar(value, wPerlScalar::Force_Real)); }

	void *each(pPerlLength *key_len, wPerlScalar *value) const;
	bool each(char **key, wPerlScalar *value) const;

	void reset() const;

	wPerlArrayShadow keys() const;

	void *get_pv(const void *key, pPerlLength key_len) const;
	pPerlInteger get_iv(const void *key, pPerlLength key_len) const;
	void *get_iv_as_pointer(const void *key, pPerlLength key_len) const;
	pPerlReal get_nv(const void *key, pPerlLength key_len) const;

	operator wPerlScalar () const
	    { return wPerlScalar(get_shared_ref(), wPerlScalar::Keep); }
};

class wPerlHashShadow: public wPerlHash
{
    friend class wPerlScalar;

    wPerl_ShadowInterface(Hash, HV)
};

wPerl_ShadowConversions(Hash, HV)

#ifdef HAS_TEMPLATES

template <class T> class tPerlHash
{
    protected:
	wPerlHash hash;

    public:
	bool exists(const char *key) const
	    { return hash.exists(key, strlen(key)); }
	bool exists(int key) const
	    { return hash.exists(&key, sizeof(key)); }

	T *get(const void *key, pPerlLength key_len) const
	    { return (T *)hash.get_pv(key, key_len); }
	T *get(const char *key) const
	    { return (T *)hash.get_pv(key, strlen(key)); }
	T *get(int key) const
	    { return (T *)hash.get_pv(&key, sizeof(int)); }

	T *set(const void *key, pPerlLength key_len, const T &value)
	    {
		T *p = (T *)hash.get_pv(key, key_len);

		if (p)
		{
		    *p = value;
		}
		else
		{
		    p = new (hash.set_as_bytes(key, key_len, 0, sizeof(T))) T(value);
		}

		return p;
	    }

	T *set(const char *key, const T &value)
	    { return set(key, strlen(key), value); }
	T *set(int key, const T &value)
	    { return set(&key, sizeof(int), value); }

	bool each(char **key, T **value) const
	    {
		wPerlScalar sv;
		if (hash.each(key, &sv)) {
		    *value = (T *)sv.as_bytes();
		    return true;
		}
		return false;
	    }

	void rebind(tPerlHash<T> &that)
	    { hash.rebind(that.hash); }

	void reset() const
	    { hash.reset(); }

	wPerlArrayShadow keys() const
	    { return hash.keys(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)hash; }
};

template <class T> class tPerlHashI
{
    protected:
	wPerlHash hash;

    public:
	T get(const void *key, pPerlLength key_len) const
	    { return (T)hash.get_iv(key, key_len); }
	T get(const char *key) const
	    { return (T)hash.get_iv(key, strlen(key)); }
	T get(int key) const
	    { return (T)hash.get_iv(&key, sizeof(int)); }

	void set(const void *key, pPerlLength key_len, T value)
	    { hash.set_as_integer(key, key_len, value); }
	void set(const char *key, T value)
	    { hash.set_as_integer(key, strlen(key), value); }
	void set(int key, T value)
	    { hash.set_as_integer(&key, sizeof(int), value); }

	bool each(char **key, T *value) const
	    {
		wPerlScalar sv;
		if (hash.each(key, &sv)) {
		    *value = (T)sv.as_integer();
		    return true;
		}
		return false;
	    }

	void reset() const
	    { hash.reset(); }

	wPerlArrayShadow keys() const
	    { return hash.keys(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)hash; }
};

template <class T> class tPerlHashP
{
    protected:
	wPerlHash hash;

    public:
	T get(const void *key, pPerlLength key_len) const
	    { return (T)hash.get_iv_as_pointer(key, key_len); }
	T get(const char *key) const
	    { return (T)hash.get_iv_as_pointer(key, strlen(key)); }
	T get(int key) const
	    { return (T)hash.get_iv_as_pointer(&key, sizeof(int)); }

	void set(const void *key, pPerlLength key_len, T value)
	    { hash.set_as_pointer(key, key_len, value); }
	void set(const char *key, T value)
	    { hash.set_as_pointer(key, strlen(key), value); }
	void set(int key, T value)
	    { hash.set_as_pointer(&key, sizeof(int), value); }

	bool each(char **key, T *value) const
	    {
		wPerlScalar sv;
		if (hash.each(key, &sv)) {
		    *value = (T)sv.as_pointer();
		    return true;
		}
		return false;
	    }

	void rebind(tPerlHashP<T> &that)
	    { hash.rebind(that.hash); }

	void reset() const
	    { hash.reset(); }

	wPerlArrayShadow keys() const
	    { return hash.keys(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)hash; }
};

template <class T> class tPerlHashR
{
    protected:
	wPerlHash hash;

    public:
	T get(const void *key, pPerlLength key_len) const
	    { return (T)hash.get_nv(key, key_len); }
	T get(const char *key) const
	    { return (T)hash.get_nv(key, strlen(key)); }
	T get(int key) const
	    { return (T)hash.get_nv(&key, sizeof(int)); }

	void set(const void *key, pPerlLength key_len, T value)
	    { hash.set_as_real(key, key_len, value); }
	void set(const char *key, T value)
	    { hash.set_as_real(key, strlen(key), value); }
	void set(int key, T value)
	    { hash.set_as_real(&key, sizeof(int), value); }

	bool each(char **key, T *value) const
	    {
		wPerlScalar sv;
		if (hash.each(key, &sv)) {
		    *value = (T)sv.as_real();
		    return true;
		}
		return false;
	    }

	void reset() const
	    { hash.reset(); }

	wPerlArrayShadow keys() const
	    { return hash.keys(); }

	operator wPerlScalar () const
	    { return (wPerlScalar)hash; }
};

#endif /* HAS_TEMPLATES */


class wPerl
{
    friend class wPerlScalar;
    friend class wPerlArray;
    friend class wPerlHash;
    friend class wPerlPattern;

    public:
	enum LookupModifier { DontCreate, Create };

    public:
	static wPerl *run();

    protected:
	PerlInterpreter *perl;

	SV *t_eval;
	SV *t_replace;
	SV *t_restart_search;
	SV *t_pos;
	SV *t_find;
	SV *t_find_next;
	SV *t_find_inquire;
	SV *t_find_inquire_next;
	SV *t_find_all;
	SV *t_find_all_count;
	SV *t_compile_find;
	SV *t_index;
	SV *t_rindex;
	SV *t_use;
	SV *t_substr;
	SV *t_rep_substr;
	SV *t_split;
	SV *t_remove_elements;
	SV *t_sort_array;

	bool use_locale;

    protected:
	static wPerl *running_interpreter;

    protected:
	bool internal_use(const char *module, int num, const char *functions[]) const;
	SV *internal_call(const SV *closure, int argc, const SV *argv[]) const;

    public:
	wPerl();
	virtual ~wPerl();

#define A(N) const char *arg ## N
	bool use(const char *module);
	bool use(const char *module, A(1));
	bool use(const char *module, A(1), A(2));
	bool use(const char *module, A(1), A(2), A(3));
	bool use(const char *module, A(1), A(2), A(3), A(4));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5), A(6));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5), A(6), A(7));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8), A(9));
	bool use(const char *module, A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8), A(9), A(10));
#undef A

	wPerlScalar eval(const char *expr);

	const wPerlScalar &undef();
	wPerlScalarShadow scalar(const char *name, LookupModifier should_create = DontCreate);
	wPerlArrayShadow array(const char *name, LookupModifier should_create = DontCreate);
	wPerlHashShadow hash(const char *name, LookupModifier should_create = DontCreate);
	wPerlScalarShadow subroutine(const char *name);

    private:
	wPerl &operator = (const wPerl &other);
	wPerl(const wPerl &other);
};

#endif /* _PERL_GLUE_HH */
