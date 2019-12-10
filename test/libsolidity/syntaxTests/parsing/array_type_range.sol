contract C {
    function f() public pure {
        uint[][1:] memory x;
        uint[][1:2] memory x;
        uint[1:] memory x;
        uint[1:2] memory x;
    }
}

// ----
// ParserError: (52-62): Expected array length expression.
// ParserError: (81-92): Expected array length expression.
// ParserError: (111-119): Expected array length expression.
// ParserError: (138-147): Expected array length expression.
