==== Source: A ====
uint external a;
==== Source: B ====
uint internal a;
==== Source: C ====
uint public a;
==== Source: D ====
uint private a;
// ----
// ParserError 2314: (A:5-13): Expected identifier but got 'external'
// ParserError 2314: (B:5-13): Expected identifier but got 'internal'
// ParserError 2314: (C:5-11): Expected identifier but got 'public'
// ParserError 2314: (D:5-12): Expected identifier but got 'private'
