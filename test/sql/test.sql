\pset null _null_

SET client_min_messages = warning;

CREATE TABLE test (a serial, b uri);

INSERT INTO test (b)
VALUES ('http://www.postgresql.org/'),
       ('http://www.postgresql.org/docs/devel/static/xfunc-sql.html#XFUNC-SQL-FUNCTION-ARGUMENTS'),
       ('http://www.postgresql.org:591/'),
       ('http://www.postgresql.org:80/'),
       ('https://duckduckgo.com/?q=postgresql&ia=about'),
       ('ftp://ftp.gnu.org/gnu/bison'),
       ('mailto:foo@example.com'),
       ('ssh://username@review.openstack.org:29418/openstack/nova.git'),
       ('ssh://foobar@review.openstack.org:29418/openstack/nova.git'),
       ('ssh://review.openstack.org:29418/openstack/nova.git'),
       ('http://admin:password@192.168.0.1'),
       ('http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html'),
       ('http://[1080::8:800:200C:417A]/foo'),
       ('http://host:'),
       (''),
       ('/'),
       ('foobar'),
       ('/foobar');

-- normalization test values from <https://tools.ietf.org/html/rfc3986#section-6.2.2>
INSERT INTO test (b)
VALUES ('HTTP://www.EXAMPLE.com/'),
       ('http://www.ex%41mple.com/'),
       ('eXAMPLE://a/./b/../b/%63/%7bfoo%7d');

SELECT * FROM test;

-- error cases
SELECT uri 'http://host:port/';


\x on
SELECT b AS uri,
       uri_normalize(b),
       uri_scheme(b),
       uri_userinfo(b),
       uri_host(b),
       uri_host_inet(b),
       uri_port(b),
       uri_path(b),
       uri_path_array(b),
       uri_query(b),
       uri_fragment(b)
  FROM test;
\x off

SELECT DISTINCT b FROM test ORDER BY b;


CREATE TABLE test2 (x text, y uri);
INSERT INTO test2 VALUES ('foo', 'http://www.postgresql.org/');

-- check hashing (issue petere/pguri#3)
SET enable_nestloop = off;
SET enable_mergejoin = off;
SELECT * FROM test JOIN test2 ON b = y AND a = 1;

-- check query_json
WITH a(uri) as (
VALUES ('https://duckduckgo.com/?q=postgresql&ia=about'),
       ('https://duckduckgo.com/?q=postgresql%20%22foo%20bar%22'),
       ('ftp://ftp.gnu.org/gnu/bison?a=b'),
       ('mailto:foo@example.com'),
       ('mailto:foo@example.com?subject=hello%20world'),
       ('ssh://username@review.openstack.org:29418/openstack/nova.git?what=uri'),
       (''),
       ('/'),
       ('foobar?token=1234'),
       ('/foobar'),
       (null)
)
SELECT uri,
       uri_query(uri::uri),
       uri_query_json(uri::uri),
       uri_query_jsonb(uri::uri)
  FROM a
 ORDER BY a;

