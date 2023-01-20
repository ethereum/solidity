==== Source: A ====
contract test {
    function f(uint a) public {
        uint public a;
    }
}
==== Source: B ====
contract test {
    function f(uint a) public {
        uint private a;
    }
}
==== Source: C ====
contract test {
    function f(uint a) public {
        uint external a;
    }
}
==== Source: D ====
contract test {
    function f(uint a) public {
        uint internal a;
    }
}
// ----
// ParserError 2314: (A:61-67): Expected ';' but got 'public'
// ParserError 2314: (B:61-68): Expected ';' but got 'private'
// ParserError 2314: (C:61-69): Expected ';' but got 'external'
// ParserError 2314: (D:61-69): Expected ';' but got 'internal'
