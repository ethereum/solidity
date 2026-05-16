int constant INT_MAX = 2**255 - 1;
int constant INT_MIN = -(2**255);

contract C {
    int[uint(INT_MAX)] arrayMax;
    int[uint(INT_MIN)] arrayMin;

    function testRuntimeEquivalence() public view returns (bool) {
        uint runtimeMin = uint(INT_MIN);
        uint runtimeMax = uint(INT_MAX);

        return
            arrayMin.length == runtimeMin && arrayMax.length == runtimeMax;
    }
}
// ----
// testRuntimeEquivalence() -> true
