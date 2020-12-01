contract C {
    uint256 public x;
    modifier m() {
        for (uint256 i = 0; i < 10; i++) {
            _;
            ++x;
            return;
        }
    }

    function f() public m m m returns (uint) {
        for (uint256 i = 0; i < 10; i++) {
            ++x;
            return 42;
        }
    }
}
// ----
// x() -> 0
// f() -> 42
// x() -> 4
