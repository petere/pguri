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

CREATE FUNCTION hash_uri(uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE sql
    AS $f$ SELECT hashtext(cast($1 AS text)); $f$;

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
    FOR TYPE uri USING hash AS
        OPERATOR 1 =,
        FUNCTION 1 hash_uri(uri);
