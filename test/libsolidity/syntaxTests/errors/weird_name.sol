error error(uint a);
contract C {
    function f() public pure {
        revert error(2);
    }
}
// ----
// ParserError 2314: (6-11): Expected identifier but got 'error'
