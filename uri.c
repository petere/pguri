#include <postgres.h>
#include <access/hash.h>
#include <catalog/pg_type.h>
#include <fmgr.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/builtins.h>
#include <utils/inet.h>

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
		default:
			elog(ERROR, "liburiparser error code %d", state.errorCode);
	}
}

static char *
uri_to_string(UriUriA *urip)
{
	int rc;
	int charsRequired;
	char *ret;

	if ((rc = uriToStringCharsRequiredA(urip, &charsRequired)) != URI_SUCCESS)
		elog(ERROR, "uriToStringCharsRequiredA() failed: error code %d", rc);
	charsRequired++;

	ret = palloc(charsRequired);
	if ((rc = uriToStringA(ret, urip, charsRequired, NULL)) != URI_SUCCESS)
		elog(ERROR, "uriToStringA() failed: error code %d", rc);

	return ret;
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
		PG_RETURN_INET_P(DirectFunctionCall1(inet_in, CStringGetDatum(tmp)));
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
		PG_RETURN_INET_P(DirectFunctionCall1(inet_in, CStringGetDatum(tmp)));
	}
	else
	{
		uriFreeUriMembersA(&uri);
		PG_RETURN_NULL();
	}
}

PG_FUNCTION_INFO_V1(uri_port);
Datum
uri_port(PG_FUNCTION_ARGS)
{
	Datum arg = PG_GETARG_DATUM(0);
	char *s = TextDatumGetCString(arg);
	UriUriA uri;
	const char *p;

	parse_uri(s, &uri);
	if (!uri.portText.first || !uri.portText.afterLast
		|| uri.portText.afterLast == uri.portText.first)
		PG_RETURN_NULL();
	p = pnstrdup(uri.portText.first, uri.portText.afterLast - uri.portText.first);
	uriFreeUriMembersA(&uri);
	PG_RETURN_INT32(strtol(p, NULL, 10));
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
		PG_RETURN_ARRAYTYPE_P(makeArrayResult(astate, CurrentMemoryContext));
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
	char *ret;

	parse_uri(s, &uri);

	if ((rc = uriNormalizeSyntaxA(&uri)) != URI_SUCCESS)
		elog(ERROR, "uriNormalizeSyntaxA() failed: error code %d", rc);

	ret = uri_to_string(&uri);

	uriFreeUriMembersA(&uri);

	PG_RETURN_URI_P((uritype *) cstring_to_text(ret));
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
		int x = strncmp(a.first, b.first,
						Min(a.afterLast - a.first, b.afterLast - b.first));
		if (x == 0)
			return (a.afterLast - a.first) - (b.afterLast - b.first);
		return x;
	}
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
	parse_uri(sa, &ub);

	if (res == 0)
		res = cmp_text_range(ua.scheme, ub.scheme);
	if (res == 0)
		res = cmp_text_range(ua.hostText, ub.hostText);
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

	chars_required = (VARSIZE(arg) - 4) * (normalize_breaks ? 6 : 3) + 1;
	ret = palloc(chars_required);
	uriEscapeExA(VARDATA(arg),
				 VARDATA(arg) + VARSIZE(arg) - 4,
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

PG_FUNCTION_INFO_V1(uri_add_base_uri);
Datum
uri_add_base_uri(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);

	UriUriA absoluteDest;
	UriUriA relativeSource;
	UriUriA absoluteBase;

	int rc;
	char *ret;

	parse_uri(TextDatumGetCString(arg1), &relativeSource);
	parse_uri(TextDatumGetCString(arg2), &absoluteBase);

	if ((rc = uriAddBaseUriA(&absoluteDest, &relativeSource, &absoluteBase)) != URI_SUCCESS)
	{
		uriFreeUriMembersA(&absoluteDest);
		if (rc == URI_ERROR_ADDBASE_REL_BASE)
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("given base is not absolute")));
		else
			elog(ERROR, "uriAddBaseUriA() failed: error code %d", rc);
	}

	ret = uri_to_string(&absoluteDest);
	uriFreeUriMembersA(&absoluteDest);
	PG_RETURN_URI_P((uritype *) cstring_to_text(ret));
}

PG_FUNCTION_INFO_V1(uri_remove_base_uri);
Datum
uri_remove_base_uri(PG_FUNCTION_ARGS)
{
	Datum arg1 = PG_GETARG_DATUM(0);
	Datum arg2 = PG_GETARG_DATUM(1);
	bool domain_root_mode = PG_GETARG_BOOL(2);

	UriUriA dest;
	UriUriA absoluteSource;
	UriUriA absoluteBase;

	int rc;
	char *ret;

	parse_uri(TextDatumGetCString(arg1), &absoluteSource);
	parse_uri(TextDatumGetCString(arg2), &absoluteBase);

	if ((rc = uriRemoveBaseUriA(&dest, &absoluteSource, &absoluteBase, domain_root_mode)) != URI_SUCCESS)
	{
		uriFreeUriMembersA(&dest);
		if (rc == URI_ERROR_REMOVEBASE_REL_BASE)
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("given base is not absolute")));
		else if (rc == URI_ERROR_REMOVEBASE_REL_SOURCE)
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("given source is not absolute")));
		else
			elog(ERROR, "uriRemoveBaseUriA() failed: error code %d", rc);
	}

	ret = uri_to_string(&dest);
	uriFreeUriMembersA(&dest);
	PG_RETURN_URI_P((uritype *) cstring_to_text(ret));
}
