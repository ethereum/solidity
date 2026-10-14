contract C {
    uint256[2] small;
    uint256[4] large;
    uint256 transient t;

    function setAndClear() external {
        large = [1,2,3,4];
        small = [10, 20];
        t = 99;

        delete t;
        assert(t == 0);
        large = small;
    }

    function getLarge() external view returns (uint256[4] memory) {
        return large;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// setAndClear() ->
// gas irOptimized: 124680
// gas legacy: 127807
// gas legacyOptimized: 124828
// gas ssaCFGOptimized: 124673
// getLarge() -> 10, 20, 0, 0
