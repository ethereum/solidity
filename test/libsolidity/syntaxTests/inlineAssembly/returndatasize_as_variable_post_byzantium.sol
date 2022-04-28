contract C {
    function f() public view {
        uint returndatasize;
        returndatasize;
        assembly {
            returndatasize := 2
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// ParserError 6272: (143-145): Cannot assign to builtin function "returndatasize".
