contract Foo {
    function f1() returns (string calldata calldata) {}
    function f2() returns (string calldata memory) {}
    function f3() returns (string calldata storage) {}
    function f4() returns (string memory calldata) {}
    function f5() returns (string memory memory) {}
    function f6() returns (string memory storage) {}
    function f7() returns (string storage calldata) {}
    function f8() returns (string storage memory) {}
    function f9() returns (string storage storage) {}
}
// ----
// ParserError 3548: (58-66): Location already specified.
// ParserError 3548: (114-120): Location already specified.
// ParserError 3548: (168-175): Location already specified.
// ParserError 3548: (221-229): Location already specified.
// ParserError 3548: (275-281): Location already specified.
// ParserError 3548: (327-334): Location already specified.
// ParserError 3548: (381-389): Location already specified.
// ParserError 3548: (436-442): Location already specified.
// ParserError 3548: (489-496): Location already specified.
