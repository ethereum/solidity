contract C {
    function f() public pure {
        while (true) unchecked {

        }
    }
}
// ----
// ParserError 5296: (65-74): "unchecked" blocks can only be used inside regular blocks.
