CREATE FUNCTION uri_query_json(uri) RETURNS json
    IMMUTABLE
    STRICT
    LANGUAGE C
    AS '$libdir/uri';

