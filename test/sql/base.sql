SELECT uri_add_base_uri('../TWO', 'file:///one/two/three');
SELECT uri_add_base_uri('/one/TWO', 'file:///one/two/three');
SELECT uri_add_base_uri('../TWO', 'http://www.example.com/one/two/three');
SELECT uri_add_base_uri('/one/TWO', 'http://www.example.com/one/two/three');
SELECT uri_add_base_uri('http://www.example.com/', 'file:///one/two/three');  -- nonsense, but works
SELECT uri_add_base_uri('http://www.example.com/', '../TWO');  -- error
SELECT uri_add_base_uri('mailto:foo@example.com', 'http://www.example.com/');

SELECT uri_remove_base_uri('file:///one/TWO', 'file:///one/two/three', false);
SELECT uri_remove_base_uri('http://www.example.com/one/TWO', 'http://www.example.com/one/two/three', false);
SELECT uri_remove_base_uri('http://www.example.com/one/TWO', 'http://www.example.net/one/two/three', false);
SELECT uri_remove_base_uri('file:///one/TWO', 'http://www.example.net/one/two/three', false);

SELECT uri_remove_base_uri('file:///one/TWO', 'file:///one/two/three', true);
SELECT uri_remove_base_uri('http://www.example.com/one/TWO', 'http://www.example.com/one/two/three', true);
SELECT uri_remove_base_uri('http://www.example.com/one/TWO', 'http://www.example.net/one/two/three', true);
SELECT uri_remove_base_uri('file:///one/TWO', 'http://www.example.net/one/two/three', true);

SELECT uri_remove_base_uri('../TWO', 'file:///one/two/three', false);  -- error
SELECT uri_remove_base_uri('file:///one/TWO', '../three', false);  -- error
