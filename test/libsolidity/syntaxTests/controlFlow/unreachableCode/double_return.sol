contract C {
    function f() public pure returns (uint) {
        return 0;
        return 0;
    }
}
// ----
// Warning 5740: (85-93='return 0'): Unreachable code.
