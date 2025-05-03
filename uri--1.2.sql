-- pguri version 1.2

SET client_min_messages = warning;

CREATE TYPE uri;

CREATE FUNCTION uri_in(cstring) RETURNS uri
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_out(uri) RETURNS cstring
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE TYPE uri (
    INTERNALLENGTH = -1,
    INPUT = uri_in,
    OUTPUT = uri_out
);


CREATE CAST (uri AS text) WITH INOUT AS ASSIGNMENT;
CREATE CAST (text AS uri) WITH INOUT AS ASSIGNMENT;


CREATE FUNCTION uri_scheme(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_userinfo(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_host(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_host_inet(uri) RETURNS inet
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_port(uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_query(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_fragment(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_path(uri) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_path_array(uri) RETURNS text[]
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';


CREATE FUNCTION uri_normalize(uri) RETURNS uri
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';


CREATE FUNCTION uri_lt(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_le(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_eq(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_ne(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_ge(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_gt(uri, uri) RETURNS boolean
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_cmp(uri, uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_hash(uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE OPERATOR < (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = >,
    NEGATOR = >=,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = uri_lt
);

CREATE OPERATOR <= (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = >=,
    NEGATOR = >,
    RESTRICT = scalarltsel,
    JOIN = scalarltjoinsel,
    PROCEDURE = uri_le
);

CREATE OPERATOR = (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = =,
    NEGATOR = <>,
    RESTRICT = eqsel,
    JOIN = eqjoinsel,
    HASHES,
    MERGES,
    PROCEDURE = uri_eq
);

CREATE OPERATOR <> (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = <>,
    NEGATOR = =,
    RESTRICT = neqsel,
    JOIN = neqjoinsel,
    PROCEDURE = uri_ne
);

CREATE OPERATOR >= (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = <=,
    NEGATOR = <,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = uri_ge
);

CREATE OPERATOR > (
    LEFTARG = uri,
    RIGHTARG = uri,
    COMMUTATOR = <,
    NEGATOR = <=,
    RESTRICT = scalargtsel,
    JOIN = scalargtjoinsel,
    PROCEDURE = uri_gt
);

CREATE OPERATOR CLASS uri_ops
    DEFAULT FOR TYPE uri USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       uri_cmp(uri, uri);

CREATE OPERATOR CLASS uri_ops_hash
    DEFAULT FOR TYPE uri USING hash AS
        OPERATOR        1       =,
        FUNCTION        1       uri_hash(uri);

CREATE FUNCTION uri_escape(text, space_to_plus boolean DEFAULT false, normalize_breaks boolean DEFAULT false) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE FUNCTION uri_unescape(text, plus_to_space boolean DEFAULT false, break_conversion boolean DEFAULT false) RETURNS text
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

-- version 1.1

CREATE FUNCTION uri_query_json(uri) RETURNS json
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

-- version 1.2

CREATE FUNCTION uri_query_jsonb(uri) RETURNS jsonb
    IMMUTABLE STRICT LANGUAGE sql
    AS $$ select uri_query_json($1)::jsonb $$;
