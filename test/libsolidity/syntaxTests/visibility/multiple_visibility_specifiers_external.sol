==== Source: A ====
contract C {
    uint external external x;
}
==== Source: B ====
contract C {
    uint internal external x;
}
==== Source: C ====
contract C {
    uint external internal x;
}
// ----
// ParserError 2314: (A:22-30): Expected identifier but got 'external'
// ParserError 2314: (B:31-39): Expected identifier but got 'external'
// ParserError 2314: (C:22-30): Expected identifier but got 'external'
