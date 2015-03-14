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

- `uri_query(uri) returns text`

    Extracts the query part of a URI (roughly speaking, everything
    after the `?`).  If there is no query part, returns null.

- `uri_fragment(uri) returns text`

    Extracts the fragment part of a URI (roughly speaking, everything
    after the `#`).  If there is no fragment part, returns null.
