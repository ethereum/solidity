==== Source: A ====
contract test {
    function f1(uint indexed a) public returns (uint) { }
}
==== Source: B ====
contract test {
    function f1(uint a) public returns (uint indexed) { }
}
// ----
// ParserError 2314: (A:37-44): Expected ',' but got 'indexed'
// ParserError 2314: (B:61-68): Expected ',' but got 'indexed'
