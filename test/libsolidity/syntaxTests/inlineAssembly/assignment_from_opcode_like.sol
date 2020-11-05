contract C {
    function f() public pure {
        uint mload;
        assembly {
            let x := mload
        }
    }
}
// ----
// ParserError 7104: (118-119): Builtin function "mload" must be called.
