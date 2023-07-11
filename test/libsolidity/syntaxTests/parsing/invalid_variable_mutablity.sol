==== Source: A ====
contract test {
    function f(uint a) public {
        uint constant a;
    }
}
==== Source: B ====
contract test {
    function f(uint a) public {
        uint immutable a;
    }
}
// ----
// ParserError 2314: (A:61-69): Expected ';' but got 'constant'
// ParserError 2314: (B:61-70): Expected ';' but got 'immutable'