contract C {
    function f() public pure {
        assembly {
            let x := hex"12__34";
        }
    }
}
// ----
// ParserError 1465: (84-92): Illegal token: Invalid use of number separator '_'.
