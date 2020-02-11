contract test {
    function f(uint k) public returns(uint) {
        return k;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 9 -> 9
