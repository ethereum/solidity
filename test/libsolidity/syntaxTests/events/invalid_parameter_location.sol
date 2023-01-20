==== Source: A ====
contract test {
    event e1(string storage a);
}
==== Source: B ====
contract test {
    event e1(string memory a);
}
==== Source: C ====
contract test {
    event e1(string calldata a);
}
// ----
// ParserError 2314: (A:36-43): Expected ',' but got 'storage'
// ParserError 2314: (B:36-42): Expected ',' but got 'memory'
// ParserError 2314: (C:36-44): Expected ',' but got 'calldata'
