error error(uint a);
contract C {
    function f() public pure {
        revert error(2);
    }
}
// ----
// f() -> FAILURE, hex"b48fb6cf", hex"0000000000000000000000000000000000000000000000000000000000000002"
