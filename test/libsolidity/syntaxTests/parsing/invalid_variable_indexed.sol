==== Source: A ====
uint indexed a;
==== Source: B ====
contract test {
    function f(uint a) public {
        uint indexed a;
    }
}
==== Source: C ====
contract test {
    uint indexed a;
}
// ----
// ParserError 2314: (A:5-12): Expected identifier but got 'indexed'
// ParserError 2314: (B:61-68): Expected ';' but got 'indexed'
// ParserError 2314: (C:25-32): Expected identifier but got 'indexed'