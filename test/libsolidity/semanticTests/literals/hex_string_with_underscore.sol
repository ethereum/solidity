contract C {
    function f() public pure returns(bytes memory) {
        return hex"12_34_5678_9A";
    }
}
// ----
// f() -> 32, 5, left(0x123456789A)
