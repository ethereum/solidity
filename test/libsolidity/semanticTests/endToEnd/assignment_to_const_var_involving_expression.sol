contract C {
    uint constant x = 0x123 + 0x456;

    function f() public returns(uint) {
        return x + 1;
    }
}

// ----
// f() -> 1402
