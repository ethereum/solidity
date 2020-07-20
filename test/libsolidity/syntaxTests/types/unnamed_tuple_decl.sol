contract C {
    function f() internal pure {}
    function g() internal pure returns (uint) { return 1; }
    function h() internal pure returns (uint, uint) { return (1, 2); }

    function test() internal pure {
        () = f();
        () = g();
        (,) = h();
    }
}

// ----
// ParserError 6933: (224-225): Expected primary expression.
