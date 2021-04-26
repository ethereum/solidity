contract c {
    function f(uint256 a) public returns (uint256) {
        return a;
    }

    function test(uint256 a, uint256 b)
        external
        returns (uint256 r_a, uint256 r_b)
    {
        r_a = f(a + 7);
        r_b = b;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// test(uint256,uint256): 2, 3 -> 9, 3
