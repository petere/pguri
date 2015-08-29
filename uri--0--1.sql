CREATE FUNCTION uri_hash(uri) RETURNS integer
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

CREATE OPERATOR CLASS uri_ops_hash
    DEFAULT FOR TYPE uri USING hash AS
        OPERATOR        1       =,
        FUNCTION        1       uri_hash(uri);
