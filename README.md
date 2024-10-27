`uri` type for PostgreSQL
=========================

https://twitter.com/pvh/status/567395527357001728

This is an extension for PostgreSQL that provides a `uri` data type.
Advantages over using plain `text` for storing URIs include:

- URI syntax checking
- functions for extracting URI components
- human-friendly sorting

The actual URI parsing is provided by the
[uriparser](http://uriparser.sourceforge.net/) library, which supports
URI syntax as per [RFC 3986](http://tools.ietf.org/html/rfc3986).

Note that this might not be the right data type to use if you want to
store user-provided URI data, such as HTTP referrers, since they might
contain arbitrary junk.

Installation
------------

You need to have the above-mentioned `uriparser` library installed.
It is included in many operating system distributions and package
management systems.  `pkg-config` will be used to find it.  I
recommend at least version 0.8.0.  Older versions will also work, but
they apparently contain some bugs and might fail to correctly accept
or reject URI syntax corner cases.  This is mainly a problem if your
application needs to be robust against junk input.

To build and install this module:

    make
    make install

or selecting a specific PostgreSQL installation:

    make PG_CONFIG=/some/where/bin/pg_config
    make PG_CONFIG=/some/where/bin/pg_config install

And finally inside the database:

    CREATE EXTENSION uri;

Using
-----

This module provides a data type `uri` that you can use like a normal
type.  For example:

```sql
CREATE TABLE links (
    id int PRIMARY KEY,
    link uri
);

INSERT INTO links VALUES (1, 'https://github.com/petere/pguri');
```

A number of functions are provided to extract parts of a URI:

- `uri_scheme(uri) returns text`

    Extracts the scheme of a URI, for example `http` or `ftp` or
    `mailto`.

- `uri_userinfo(uri) returns text`

    Extracts the user info part of a URI.  This is normally a user
    name, but could also be of the form `username:password`.  If the
    URI does not contain a user info part, then this will return null.

- `uri_host(uri) returns text`

    Extracts the host of a URI, for example `www.example.com` or
    `192.168.0.1`.  (For IPv6 addresses, the brackets are not included
    here.)  If there is no host, the return value is null.

- `uri_host_inet(uri) returns inet`

    If the host is a raw IP address, then this will return it as an
    `inet` datum.  Otherwise (not an IP address or no host at all),
    the return value is null.

- `uri_port(uri) returns integer`

    Extracts the port of a URI as an integer, for example `5432`.  If
    no port is specified, the return value is null.

- `uri_path(uri) returns text`

    Extracts the path component of a URI.  Logically, a URI always
    contains a path.  The return value can be an empty string but
    never null.

- `uri_path_array(uri) returns text[]`

    Returns the path component of a URI as an array, with the path
    split at the slash characters.  This is probably not as useful as
    the `uri_path` function, but it is provided here because the
    `uriparser` library exposes it.

- `uri_query(uri) returns text`

    Extracts the query part of a URI (roughly speaking, everything
    after the `?`).  If there is no query part, returns null.

- `uri_query_json(uri) returns json`

    Converts the query part of a URI (roughly speaking, everything
    after the `?`) into a JSON object. This provides for easier access
    to individial query parameters. If there is no query part, returns null.

- `uri_query_jsonb(uri) returns jsonb`

    Converts the query part of a URI (roughly speaking, everything
    after the `?`) into a JSONB object. This provides for easier access
    to individial query parameters. If there is no query part, returns null.

- `uri_fragment(uri) returns text`

    Extracts the fragment part of a URI (roughly speaking, everything
    after the `#`).  If there is no fragment part, returns null.

Other functions:

- `uri_normalize(uri) returns uri`

    Performs syntax-based normalization of the URI.  This includes
    case normalization, percent-encoding normalization, and removing
    redundant `.` and `..` path segments.  See
    [RFC 3986 section 6.2.2](http://tools.ietf.org/html/rfc3986#section-6.2.2)
    for the full details.

    Note that this module (and similar modules in other programming
    languages) compares URIs for equality in their original form,
    without normalization.  If you want to consider distinct URIs
    without regard for mostly irrelevant syntax differences, pass them
    through this function.

- `uri_escape(text, space_to_plus boolean DEFAULT false, normalize_breaks boolean DEFAULT false) returns text`

    Percent-encodes all reserved characters from the text.  This can
    be useful for constructing URIs from strings.

    If `space_to_plus` is true, then spaces are replaced by plus
    signs.  If `normalize_breaks` is true, then line breaks are
    converted to CR LF pairs (and subsequently percent-encoded).  Note
    that these two conversions come from the HTML standard for
    encoding form data but are not part of the specification for URIs.

- `uri_unescape(text, plus_to_space boolean DEFAULT false, break_conversion boolean DEFAULT false) returns text`

    Decodes all percent-encodings in the text.

    If `plus_to_space` is true, then plus signs are converted to
    spaces.  If `break_conversion` is true, then CR LF pairs are
    converted to simple newlines (`\n`).
