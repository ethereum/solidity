contract c {
    function mul(fixed a, fixed b) public pure returns (fixed c) {
        return a * b;
    }
}
// ----
// mul(fixed128x18,fixed128x18): 200, 10000000000000000000000 -> 2000000
