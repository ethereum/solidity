contract C {
    function max() public pure returns (uint8) {
        return (type(uint8)).max;
    }
}
// ----
