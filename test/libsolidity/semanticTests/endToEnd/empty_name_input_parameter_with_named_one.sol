contract test {
    function f(uint, uint k) public returns(uint ret_k, uint ret_g) {
        uint g = 8;
        ret_k = k;
        ret_g = g;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256,uint256): 5, 9 -> 9, 8
