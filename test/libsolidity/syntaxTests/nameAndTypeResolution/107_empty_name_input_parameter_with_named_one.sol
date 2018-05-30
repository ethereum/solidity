contract test {
    function f(uint, uint k) public returns (uint ret_k) {
        return k;
    }
}
// ----
// Warning: (20-98): Function state mutability can be restricted to pure
