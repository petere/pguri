CREATE OR REPLACE FUNCTION uri_query_jsonb(uri) RETURNS jsonb
    IMMUTABLE STRICT LANGUAGE sql
    AS $$ select uri_query_json($1)::jsonb $$;

