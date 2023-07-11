contract test {
    function f(bool cond, uint v) public returns (uint a, uint b) {
        cond ? a = v : b = v;
    }
}
// ----
// f(bool,uint256): true, 20 -> 20, 0
// f(bool,uint256): false, 20 -> 0, 20
