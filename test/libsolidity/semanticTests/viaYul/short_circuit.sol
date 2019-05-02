contract C {
    function or(uint x) public returns (bool t, uint y) {
        t = (x == 0 || ((x = 8) > 0));
        y = x;
    }
    function and(uint x) public returns (bool t, uint y) {
        t = (x == 0 && ((x = 8) > 0));
        y = x;
    }
}
// ====
// compileViaYul: true
// ----
// or(uint256): 0 -> true, 0
// and(uint256): 0 -> true, 8
// or(uint256): 1 -> true, 8
// and(uint256): 1 -> false, 1
