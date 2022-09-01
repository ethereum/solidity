contract C {
    function intern() public returns (uint256) {
        function (uint) internal returns (uint) x;
        x(2);
        return 7;
    }

    function extern() public returns (uint256) {
        function (uint) external returns (uint) x;
        x(2);
        return 7;
    }
}
// ====
// compileToEwasm: also
// ----
// intern() -> FAILURE, hex"4e487b71", 0x51 # This should throw exceptions #
// extern() -> FAILURE
