function suffix(uint) pure suffix returns (uint) {}

contract C {
    function suffix(uint) internal pure returns (uint) {}

    function run() public pure returns (uint) {
        return 1 suffix;
    }
}
// ----
// Warning 2519: (70-123): This declaration shadows an existing declaration.
// TypeError 4438: (190-196): The literal suffix must be either a subdenomination or a file-level suffix function.
