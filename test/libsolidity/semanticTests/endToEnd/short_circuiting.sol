contract test {
    function run(uint x) public returns(uint y) {
        x == 0 || ((x = 8) > 0);
        return x;
    }
}

// ====
// compileViaYul: also
// ----
// run(uint256): 0 -> 0
// run(uint256): 1 -> 8
