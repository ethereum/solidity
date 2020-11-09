contract C {
    function f() public pure {
        uint mload;
        assembly {
            let x := mload
        }
    }
}
// ----
// ParserError 2314: (118-119): Expected '(' but got '}'
