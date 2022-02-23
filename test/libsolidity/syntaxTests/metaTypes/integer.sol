contract Test {
    function basic() public pure {
        uint uintMax = type(uint).max;
        uintMax;
        int intMax = type(int).max;
        intMax;
        uint uintMin = type(uint).min;
        uintMin;
        int intMin = type(int).min;
        intMin;
    }
}
// ----
