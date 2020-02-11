contract test {
    function f(uint n) public returns(uint nfac) {
        nfac = 1;
        uint i;
        for (i = 2; i <= n; i++)
            nfac *= i;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 1
// f(uint256): 1 -> 1
// f(uint256): 2 -> 2
// f(uint256): 3 -> 6
// f(uint256): 4 -> 24
