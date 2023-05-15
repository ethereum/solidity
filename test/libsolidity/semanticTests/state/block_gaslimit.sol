contract C {
    function f() public returns (uint) {
        return block.gaslimit;
    }
}
// ----
// f() -> 20000000
// f() -> 20000000
// f() -> 20000000
