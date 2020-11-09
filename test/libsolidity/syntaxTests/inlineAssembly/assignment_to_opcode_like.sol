contract C {
    function f() public pure {
        uint mload;
        assembly {
            mload := 1
        }
    }
}
// ----
// ParserError 2314: (101-103): Expected '(' but got ':='
