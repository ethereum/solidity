contract C {
    function f() public pure {
        uint mload;
        assembly {
            let x := mload
        }
    }
}
// ----
// ParserError 7104: (104-109): Builtin function "mload" must be called.
