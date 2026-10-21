contract C {
    uint constant ONE = 1;
    int constant MINUS_ONE = -1;
    uint constant LEFT_SHIFT = ONE << 0x100000000;
    uint constant RIGHT_SHIFT_POSITIVE = ONE >> 0x100000000;
    int constant RIGHT_SHIFT_NEGATIVE = MINUS_ONE >> 0x100000000;
    uint[LEFT_SHIFT + 1] left;
    uint[RIGHT_SHIFT_POSITIVE + 1] rightPositive;
    uint[RIGHT_SHIFT_NEGATIVE * -1] rightNegative;

    function testEquivalence() public view returns (bool) {
        uint leftRuntime = ONE << 0x100000000;
        uint rightPositiveRuntime = ONE >> 0x100000000;
        int rightNegativeRuntime = MINUS_ONE >> 0x100000000;

        return
            leftRuntime == LEFT_SHIFT &&
            rightPositiveRuntime == RIGHT_SHIFT_POSITIVE &&
            rightNegativeRuntime == RIGHT_SHIFT_NEGATIVE &&
            left.length == 1 &&
            rightPositive.length == 1 &&
            rightNegative.length == 1;
    }
}
// ----
// testEquivalence() -> true
