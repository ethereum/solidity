contract C {
    modifier repeat(uint256 count) {
        uint256 i;
        for (i = 0; i < count; ++i) _;
    }

    function f() public repeat(10) returns (uint256 r) {
        r += 1;
    }
}

// ----
// f() -> 10
