contract C {
    function f() public pure {
        assembly {
            let x := bin"01xy";
        }
    }
}
// ----
// ParserError 1465: (84-88): Illegal token: Expected numbers of bits multiple of 8.
