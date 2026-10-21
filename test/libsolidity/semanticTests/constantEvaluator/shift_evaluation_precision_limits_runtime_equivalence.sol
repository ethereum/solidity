contract C {
    int constant ONE = 1;
    int constant MINUS_ONE = -1;
    int constant LEFT_SHIFT = MINUS_ONE << 5000;
    int constant RIGHT_SHIFT_POSITIVE = ONE >> 5000;
    int constant RIGHT_SHIFT_NEGATIVE = MINUS_ONE >> 5000;
    uint[LEFT_SHIFT + 1] left;
    uint[RIGHT_SHIFT_POSITIVE + 1] rightPositive;
    uint[RIGHT_SHIFT_NEGATIVE * -1] rightNegative;

    function testRuntimeEquivalence() public view returns (bool) {
        int leftRuntime = MINUS_ONE << 5000;
        int rightPositiveRuntime = ONE >> 5000;
        int rightNegativeRuntime = MINUS_ONE >> 5000;

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
// testRuntimeEquivalence() -> true
