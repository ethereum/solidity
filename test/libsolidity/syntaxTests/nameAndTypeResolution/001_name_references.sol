contract test {
    uint256 variable;
    function f(uint256) public returns (uint out) { f(variable); test; out; }
}
// ----
// Warning: (103-107): Statement has no effect.
