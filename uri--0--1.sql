CREATE FUNCTION uri_hash(uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

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
