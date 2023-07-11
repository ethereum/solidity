==== Source: A ====
contract test {
    string memory a;
}
==== Source: B ====
contract test {
    string calldata a;
}
// ----
// ParserError 2314: (A:27-33): Expected identifier but got 'memory'
// ParserError 2314: (B:27-35): Expected identifier but got 'calldata'
