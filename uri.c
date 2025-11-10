#include <postgres.h>
#include <access/hash.h>
#include <catalog/pg_type.h>
#include <fmgr.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/builtins.h>
#include <utils/inet.h>
#include <utils/json.h>

#include <uriparser/Uri.h>


PG_MODULE_MAGIC;


typedef struct varlena uritype;


#define DatumGetUriP(X)		((uritype *) PG_DETOAST_DATUM(X))
#define DatumGetUriPP(X)	((uritype *) PG_DETOAST_DATUM_PACKED(X))
#define UriPGetDatum(X)		PointerGetDatum(X)

#define PG_GETARG_URI_P(n)	DatumGetUriP(PG_GETARG_DATUM(n))
#define PG_GETARG_URI_PP(n)	DatumGetUriPP(PG_GETARG_DATUM(n))
#define PG_RETURN_URI_P(x)	PG_RETURN_POINTER(x)


static void
parse_uri(const char *s, UriUriA *urip)
{
	UriParserStateA state;

	state.uri = urip;
	uriParseUriA(&state, s);

	switch (state.errorCode)
	{
		case URI_SUCCESS:
			return;
		case URI_ERROR_SYNTAX:
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
					 errmsg("invalid input syntax for type uri at or near \"%s\"",
							state.errorPos)));
			break;
		default:
			elog(ERROR, "liburiparser error code %d", state.errorCode);
	}
}

PG_FUNCTION_INFO_V1(uri_in);
Datum
uri_in(PG_FUNCTION_ARGS)
{
	char *s = PG_GETARG_CSTRING(0);
	uritype *vardata;
	UriUriA uri;

	parse_uri(s, &uri);
	uriFreeUriMembersA(&uri);

	vardata = (uritype *) cstring_to_text(s);
	PG_RETURN_URI_P(vardata);
}

PG_FUNCTION_INFO_V1(uri_out);
Datum
uri_out(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);

	PG_RETURN_CSTRING(TextDatumGetCString(arg));
}

static text *
uri_text_range_to_text(UriTextRangeA r)
{
	if (!r.first || !r.afterLast)
		return NULL;

	return cstring_to_text_with_len(r.first, r.afterLast - r.first);
}

PG_FUNCTION_INFO_V1(uri_scheme);
Datum
uri_scheme(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	text *result;

	parse_uri(s, &uri);
	result = uri_text_range_to_text(uri.scheme);
	uriFreeUriMembersA(&uri);
	if (result)
		PG_RETURN_TEXT_P(result);
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(uri_userinfo);
Datum
uri_userinfo(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	text *result;

	parse_uri(s, &uri);
	result = uri_text_range_to_text(uri.userInfo);
	uriFreeUriMembersA(&uri);
	if (result)
		PG_RETURN_TEXT_P(result);
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(uri_host);
Datum
uri_host(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	text *result;

	parse_uri(s, &uri);
	result = uri_text_range_to_text(uri.hostText);
	uriFreeUriMembersA(&uri);
	if (result)
		PG_RETURN_TEXT_P(result);
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(uri_host_inet);
Datum
uri_host_inet(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;

	parse_uri(s, &uri);
	if (uri.hostData.ip4)
	{
		unsigned char *data = uri.hostData.ip4->data;
		char *tmp = palloc(16);
		snprintf(tmp, 16, "%u.%u.%u.%u", data[0], data[1], data[2], data[3]);
		uriFreeUriMembersA(&uri);
		PG_RETURN_DATUM(DirectFunctionCall1(inet_in, CStringGetDatum(tmp)));
	}
	else if (uri.hostData.ip6)
	{
		unsigned char *data = uri.hostData.ip6->data;
		char *tmp = palloc(40);
		snprintf(tmp, 40, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				 data[0], data[1], data[2], data[3],
				 data[4], data[5], data[6], data[7],
				 data[8], data[9], data[10], data[11],
				 data[12], data[13], data[14], data[15]);
		uriFreeUriMembersA(&uri);
		PG_RETURN_DATUM(DirectFunctionCall1(inet_in, CStringGetDatum(tmp)));
	}
	else
	{
		uriFreeUriMembersA(&uri);
		PG_RETURN_NULL();
	}
}

static int
_uri_port_num(UriUriA *urip)
{
	if (!urip->portText.first || !urip->portText.afterLast
		|| urip->portText.afterLast == urip->portText.first)
		return -1;
	return strtol(pnstrdup(urip->portText.first, urip->portText.afterLast - urip->portText.first),
				 NULL, 10);
}

PG_FUNCTION_INFO_V1(uri_port);
Datum
uri_port(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	int num;

	parse_uri(s, &uri);
	num = _uri_port_num(&uri);
	uriFreeUriMembersA(&uri);
	if (num < 0)
		PG_RETURN_NULL();
	PG_RETURN_INT32(num);
}

PG_FUNCTION_INFO_V1(uri_query);
Datum
uri_query(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	text *result;

	parse_uri(s, &uri);
	result = uri_text_range_to_text(uri.query);
	uriFreeUriMembersA(&uri);
	if (result)
		PG_RETURN_TEXT_P(result);
	else
		PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(uri_query_json);
Datum
uri_query_json(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	UriQueryListA *queryList;
	int itemCount;
	StringInfoData dst;

	parse_uri(s, &uri);
	if(uriDissectQueryMallocA(&queryList, &itemCount,
	  uri.query.first, uri.query.afterLast) == URI_SUCCESS) {
	  UriQueryListA *p = queryList;
	  initStringInfo(&dst);
	  appendStringInfoChar(&dst, '{');
	  while(p) {
	    escape_json(&dst,p->key);
	    appendStringInfoChar(&dst, ':');
	    escape_json(&dst,p->value);
	    if(p->next) appendStringInfoChar(&dst,',');
	    p = p->next;
	  }
	  uriFreeQueryListA(queryList);
	  uriFreeUriMembersA(&uri);
	  appendStringInfoChar(&dst, '}');
	  PG_RETURN_TEXT_P(cstring_to_text(dst.data));
	}
	uriFreeUriMembersA(&uri);

	PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(uri_fragment);
Datum
uri_fragment(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	text *result;

	parse_uri(s, &uri);
	result = uri_text_range_to_text(uri.fragment);
	uriFreeUriMembersA(&uri);
	if (result)
		PG_RETURN_TEXT_P(result);
	else
		PG_RETURN_NULL();
}

/*
 * Defined in uriparser library, but not exported, so we keep a local version
 * here.
 */
static bool
_is_host_set(UriUriA *uri)
{
	return (uri != NULL)
		&& ((uri->hostText.first != NULL)
			|| (uri->hostData.ip4 != NULL)
			|| (uri->hostData.ip6 != NULL)
			|| (uri->hostData.ipFuture.first != NULL)
			);
}

PG_FUNCTION_INFO_V1(uri_path);
Datum
uri_path(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	StringInfoData buf;
	UriPathSegmentA *p;

	initStringInfo(&buf);

	parse_uri(s, &uri);

	if (uri.absolutePath || (_is_host_set(&uri) && uri.pathHead))
		appendStringInfoChar(&buf, '/');

	for (p = uri.pathHead; p; p = p->next)
	{
		appendBinaryStringInfo(&buf, p->text.first, p->text.afterLast - p->text.first);
		if (p->next)
			appendStringInfoChar(&buf, '/');
	}

	uriFreeUriMembersA(&uri);
	PG_RETURN_TEXT_P(cstring_to_text(buf.data));
}

PG_FUNCTION_INFO_V1(uri_path_array);
Datum
uri_path_array(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	ArrayBuildState *astate = NULL;
	UriPathSegmentA *pa;

	parse_uri(s, &uri);
	for (pa = uri.pathHead; pa; pa = pa->next)
	{
		text *piece = uri_text_range_to_text(pa->text);
		astate = accumArrayResult(astate,
								  PointerGetDatum(piece),
								  !piece,
								  TEXTOID,
								  CurrentMemoryContext);
	}
	uriFreeUriMembersA(&uri);

	if (astate)
		PG_RETURN_DATUM(makeArrayResult(astate, CurrentMemoryContext));
	else
		PG_RETURN_ARRAYTYPE_P(construct_empty_array(TEXTOID));
}

PG_FUNCTION_INFO_V1(uri_normalize);
Datum
uri_normalize(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	int rc;
	int charsRequired;
	char *ret;

	parse_uri(s, &uri);

	if ((rc = uriNormalizeSyntaxA(&uri)) != URI_SUCCESS)
		elog(ERROR, "uriNormalizeSyntaxA() failed: error code %d", rc);

	if ((rc = uriToStringCharsRequiredA(&uri, &charsRequired)) != URI_SUCCESS)
		elog(ERROR, "uriToStringCharsRequiredA() failed: error code %d", rc);
	charsRequired++;

	ret = palloc(charsRequired);
	if ((rc = uriToStringA(ret, &uri, charsRequired, NULL)) != URI_SUCCESS)
		elog(ERROR, "uriToStringA() failed: error code %d", rc);

	uriFreeUriMembersA(&uri);

	PG_RETURN_URI_P((uritype *) cstring_to_text(ret));
}

static int
strcasecmp_ascii(const char *s1, const char *s2)
{
	for (;;)
	{
		unsigned char ch1 = (unsigned char) *s1++;
		unsigned char ch2 = (unsigned char) *s2++;

		if (ch1 != ch2)
		{
			if (ch1 >= 'A' && ch1 <= 'Z')
				ch1 += 'a' - 'A';

			if (ch2 >= 'A' && ch2 <= 'Z')
				ch2 += 'a' - 'A';

			if (ch1 != ch2)
				return (int) ch1 - (int) ch2;
		}
		if (ch1 == 0)
			break;
	}
	return 0;
}

static int
strncasecmp_ascii(const char *s1, const char *s2, size_t n)
{
	while (n-- > 0)
	{
		unsigned char ch1 = (unsigned char) *s1++;
		unsigned char ch2 = (unsigned char) *s2++;

		if (ch1 != ch2)
		{
			if (ch1 >= 'A' && ch1 <= 'Z')
				ch1 += 'a' - 'A';

			if (ch2 >= 'A' && ch2 <= 'Z')
				ch2 += 'a' - 'A';

			if (ch1 != ch2)
				return (int) ch1 - (int) ch2;
		}
		if (ch1 == 0)
			break;
	}
	return 0;
}

static int
cmp_text_range(UriTextRangeA a, UriTextRangeA b)
{
	if (!a.first || !a.afterLast)
	{
		if (!b.first || !b.afterLast)
			return 0;
		else
			return -1;
	}
	else if (!b.first || !b.afterLast)
		return 1;
	else
	{
		int x = strncasecmp_ascii(a.first, b.first,
								  Min(a.afterLast - a.first, b.afterLast - b.first));
		if (x == 0)
			return (a.afterLast - a.first) - (b.afterLast - b.first);
		return x;
	}
}

static int
cmp_hosts(UriUriA *uap, UriUriA *ubp)
{
	if (!uap->hostText.first)
	{
		if (!ubp->hostText.first)
			return 0;
		else
			return -1;
	}
	else if (uap->hostData.ip4)
	{
		if (!ubp->hostText.first)
			return 1;
		else if (ubp->hostData.ip4)
			return memcmp(uap->hostData.ip4->data,
						  ubp->hostData.ip4->data,
						  sizeof(uap->hostData.ip4->data));
		else
			return -1;
	}
	else if (uap->hostData.ip6)
	{
		if (!ubp->hostText.first)
			return 1;
		else if (ubp->hostData.ip4)
			return 1;
		else if (ubp->hostData.ip6)
			return memcmp(uap->hostData.ip6->data,
						  ubp->hostData.ip6->data,
						  sizeof(uap->hostData.ip6->data));
		else
			return -1;
	}
	else
		return cmp_text_range(uap->hostText, ubp->hostText);
}

static int
_uri_cmp(Datum a, Datum b)
{
	const char *sa = TextDatumGetCString(a);
	const char *sb = TextDatumGetCString(b);
	UriUriA ua;
	UriUriA ub;
	int res = 0;

	parse_uri(sa, &ua);
	parse_uri(sb, &ub);

	if (res == 0)
		res = cmp_text_range(ua.scheme, ub.scheme);
	if (res == 0)
		res = cmp_hosts(&ua, &ub);
	if (res == 0)
		res = _uri_port_num(&ua) - _uri_port_num(&ub);
	if (res == 0)
		res = cmp_text_range(ua.userInfo, ub.userInfo);
	if (res == 0)
		res = strcasecmp_ascii(sa, sb);
	if (res == 0)
		res = strcmp(sa, sb);
	uriFreeUriMembersA(&ua);
	uriFreeUriMembersA(&ub);

	return res;
}

PG_FUNCTION_INFO_V1(uri_lt);
Datum
uri_lt(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) < 0);
}

PG_FUNCTION_INFO_V1(uri_le);
Datum
uri_le(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) <= 0);
}

PG_FUNCTION_INFO_V1(uri_eq);
Datum
uri_eq(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) == 0);
}

PG_FUNCTION_INFO_V1(uri_ne);
Datum
uri_ne(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) != 0);
}

PG_FUNCTION_INFO_V1(uri_ge);
Datum
uri_ge(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) >= 0);
}

PG_FUNCTION_INFO_V1(uri_gt);
Datum
uri_gt(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_BOOL(_uri_cmp(arg1, arg2) > 0);
}

PG_FUNCTION_INFO_V1(uri_cmp);
Datum
uri_cmp(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	PG_RETURN_INT32(_uri_cmp(arg1, arg2));
}

PG_FUNCTION_INFO_V1(uri_hash);
Datum
uri_hash(PG_FUNCTION_ARGS)
{
	uritype	   *key = PG_GETARG_URI_PP(0);
	Datum		result;

	result = hash_any((unsigned char *) VARDATA_ANY(key),
					  VARSIZE_ANY_EXHDR(key));

	/* Avoid leaking memory for toasted inputs */
	PG_FREE_IF_COPY(key, 0);

	return result;
}

PG_FUNCTION_INFO_V1(uri_escape);
Datum
uri_escape(PG_FUNCTION_ARGS)
{
	text *arg = PG_GETARG_TEXT_PP(0);
	bool space_to_plus = PG_GETARG_BOOL(1);
	bool normalize_breaks = PG_GETARG_BOOL(2);

	size_t chars_required;
	char *ret;

	chars_required = (VARSIZE_ANY_EXHDR(arg)) * (normalize_breaks ? 6 : 3) + 1;
	ret = palloc(chars_required);
	uriEscapeExA(VARDATA_ANY(arg),
				 VARDATA_ANY(arg) + VARSIZE_ANY_EXHDR(arg),
				 ret,
				 space_to_plus, normalize_breaks);

	PG_RETURN_TEXT_P(cstring_to_text(ret));
}

PG_FUNCTION_INFO_V1(uri_unescape);
Datum
uri_unescape(PG_FUNCTION_ARGS)
{
	text *arg = PG_GETARG_TEXT_PP(0);
	bool plus_to_space = PG_GETARG_BOOL(1);
	bool break_conversion = PG_GETARG_BOOL(2);

	char *s = text_to_cstring(arg);

	uriUnescapeInPlaceExA(s, plus_to_space, break_conversion);

	PG_RETURN_TEXT_P(cstring_to_text(s));
}
