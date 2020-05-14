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
// compileViaYul: also
// ----
// intern() -> FAILURE # This should throw exceptions #
// extern() -> FAILURE
