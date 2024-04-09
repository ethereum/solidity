contract C {
    function f() public pure {
        assembly {
            let x := bin"01010101__10101010";
        }
    }
}
// ----
// ParserError 1465: (84-98): Illegal token: Invalid use of number separator '_'.
