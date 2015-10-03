SELECT uri_escape('foobar');
SELECT uri_escape(':/?#[]@!$&''()*+,;=');

SELECT uri_escape('foo bar');
SELECT uri_escape('foo bar', space_to_plus := false);
SELECT uri_escape('foo bar', space_to_plus := true);

SELECT uri_escape(E'foo\nbar');
SELECT uri_escape(E'foo\nbar', normalize_breaks := false);
SELECT uri_escape(E'foo\nbar', normalize_breaks := true);

SELECT uri_unescape('foobar');
SELECT uri_unescape('%3A%2F%3F%23%5B%5D%40%21%24%26%27%28%29%2A%2B%2C%3B%3D');

SELECT uri_unescape('foo+bar');
SELECT uri_unescape('foo+bar', plus_to_space := false);
SELECT uri_unescape('foo+bar', plus_to_space := true);

SELECT uri_unescape('foo%0D%0Abar');
SELECT uri_unescape('foo%0D%0Abar', break_conversion := false);
SELECT uri_unescape('foo%0D%0Abar', break_conversion := true);
