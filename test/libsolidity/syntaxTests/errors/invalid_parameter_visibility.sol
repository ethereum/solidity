==== Source: A ====
contract C {
    error e1(uint external x);
}
==== Source: B ====
contract C {
    error e1(uint internal x);
}
==== Source: C ====
contract C {
    error e1(uint public x);
}
==== Source: D ====
contract C {
    error e1(uint private x);
}
// ----
// ParserError 2314: (A:31-39): Expected ',' but got 'external'
// ParserError 2314: (B:31-39): Expected ',' but got 'internal'
// ParserError 2314: (C:31-37): Expected ',' but got 'public'
// ParserError 2314: (D:31-38): Expected ',' but got 'private'
