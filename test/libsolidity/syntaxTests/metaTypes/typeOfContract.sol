contract Test {
    function f() public pure returns (bytes memory) {
        type(Test);
    }
}
// ----
