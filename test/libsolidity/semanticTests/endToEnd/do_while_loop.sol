contract test {
    function f(uint n) public returns(uint nfac) {
        nfac = 1;
        uint i = 2;
        do {
            nfac *= i++;
        } while (i <= n);
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 2
// f(uint256): 1 -> 2
// f(uint256): 2 -> 2
// f(uint256): 3 -> 6
// f(uint256): 4 -> 24
