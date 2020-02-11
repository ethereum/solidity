contract test {
    function f(uint n) public returns(uint nfac) {
        if (n <= 1) return 1;
        else return n * f(n - 1);
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
