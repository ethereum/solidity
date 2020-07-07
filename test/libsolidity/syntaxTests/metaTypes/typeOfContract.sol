contract Test {
    function f() public pure returns (bytes memory) {
        type(Test);
    }
}
// ----
// Warning 6133: (78-88): Statement has no effect.
