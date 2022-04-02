contract C {
    function f() public pure {
        assembly {
            let x := hex"abxy";
        }
    }
}
// ----
// ParserError 1465: (84-90='hex"ab'): Illegal token: Expected even number of hex-nibbles.
