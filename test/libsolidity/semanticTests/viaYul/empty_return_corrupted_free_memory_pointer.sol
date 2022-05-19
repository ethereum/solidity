contract C {
    function f() public {
        assembly{ mstore(0x40, sub(0, 1)) }
    }
}
// ----
// f() ->
