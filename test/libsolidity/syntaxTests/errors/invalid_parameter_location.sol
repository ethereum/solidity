==== Source: A ====
contract test {
    error e1(string storage a);
}
==== Source: B ====
contract test {
    error e1(string memory a);
}
==== Source: C ====
contract test {
    error e1(string calldata a);
}
==== Source: D ====
contract test {
    error e1(string transient a);
}
// ----
// ParserError 2314: (A:36-43): Expected ',' but got 'storage'
// ParserError 2314: (B:36-42): Expected ',' but got 'memory'
// ParserError 2314: (C:36-44): Expected ',' but got 'calldata'
// ParserError 2314: (D:46-47): Expected ',' but got identifier
