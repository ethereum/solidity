contract C {
    function f() public pure {
        uint mload;
        assembly {
            mload := 1
        }
    }
}
// ----
// ParserError 6272: (101-103): Cannot assign to builtin function "mload".
