contract C {
    modifier repeat(uint256 count) {
        uint256 i;
        for (i = 0; i < count; ++i) _;
    }

    function f() public repeat(10) returns (uint256 r) {
        r += 1;
    }
}
// via yul disabled because the return variables are
// fresh variables each time, while in the old code generator,
// they share a stack slot when the function is
// invoked multiple times via `_`.

// ====
// compileViaYul: false
// ----
// f() -> 10
