==== Source: A ====
contract test {
    event e1(uint external a);
}
==== Source: B ====
contract test {
    event e1(uint internal a);
}
==== Source: C ====
contract test {
    event e1(uint public a);
}
==== Source: D ====
contract test {
    event e1(uint private a);
}
// ----
// ParserError 2314: (A:34-42): Expected ',' but got 'external'
// ParserError 2314: (B:34-42): Expected ',' but got 'internal'
// ParserError 2314: (C:34-40): Expected ',' but got 'public'
// ParserError 2314: (D:34-41): Expected ',' but got 'private'
