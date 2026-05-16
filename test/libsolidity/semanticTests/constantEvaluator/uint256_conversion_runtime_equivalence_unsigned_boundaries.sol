contract C layout at uint(0) {
    int[uint(2**256 - 1)] array;
    function testRuntimeEquivalence() public view returns (bool) {
        uint layoutBase;
        assembly {
            layoutBase := array.slot
        }

        return array.length == 2**256 - 1 && layoutBase == 0;
    }
}
// ----
// testRuntimeEquivalence() -> true
