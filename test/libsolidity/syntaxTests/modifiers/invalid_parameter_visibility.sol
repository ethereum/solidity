==== Source: A ====
contract A {
    modifier mod(uint internal a) { _; }
}
==== Source: B ====
contract A {
    modifier mod(uint external a) { _; }
}
==== Source: C ====
contract A {
    modifier mod(uint public a) { _; }
}
==== Source: D ====
contract A {
    modifier mod(uint private a) { _; }
}
// ----
// ParserError 2314: (A:35-43): Expected ',' but got 'internal'
// ParserError 2314: (B:35-43): Expected ',' but got 'external'
// ParserError 2314: (C:35-41): Expected ',' but got 'public'
// ParserError 2314: (D:35-42): Expected ',' but got 'private'
