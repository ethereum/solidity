contract C {
    function f() public returns (uint256) {
        uint256[][] memory a = new uint256[][](0);
        return 7;
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> 7
