contract C {
    // (2**3)**4 = 4096
    // 2**(3**4) = 2417851639229258349412352
    function test_hardcode1(uint a, uint b, uint c) public returns (uint256) {
        return a**b**c;
    }

    // (3**2)**2)**2 = 6561
    // 3**(2**(2**2) = 43046721
    function test_hardcode2(uint a, uint b, uint c, uint d) public returns (uint256) {
        return a**b**c**d;
    }

    function test_invariant(uint a, uint b, uint c) public returns (bool) {
        return a**b**c == a**(b**c);
    }

    function test_literal_mix(uint a, uint b) public returns (bool) {
        return
            (a**2**b == a**(2**b)) &&
            (2**a**b == 2**(a**b)) &&
            (a**b**2 == a**(b**2));
    }

    function test_other_operators(uint a, uint b) public returns (bool) {
        return
            (a**b/25 == (a**b)/25) &&
            (a**b*3**b == (a**b)*(3**b)) &&
            (b**a**a/b**a**b == (b**(a**a))/(b**(a**b)));
     }
}
// ----
// test_hardcode1(uint256,uint256,uint256): 2, 3, 4 -> 2417851639229258349412352
// test_hardcode2(uint256,uint256,uint256,uint256): 3, 2, 2, 2 -> 43046721
// test_invariant(uint256,uint256,uint256): 2, 3, 4 -> true
// test_invariant(uint256,uint256,uint256): 3, 4, 2 -> true
// test_literal_mix(uint256,uint256): 2, 3 -> true
// test_other_operators(uint256,uint256): 2, 4 -> true
