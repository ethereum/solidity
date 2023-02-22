contract C {
    uint256 v;

    function f() public returns (bool) {
        uint256 startGas = gasleft();
        v++;
        assert(startGas > gasleft());
        return true;
    }

    function g() public returns (bool) {
        uint256 startGas = gasleft();
        assert(startGas > gasleft());
        return true;
    }
}
// ----
// f() -> true
// g() -> true
