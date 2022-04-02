contract test {
    uint256 variable;
    function f(uint256) public returns (uint out) { f(variable); test; out; }
}
// ----
// Warning 5740: (103-112='test; out'): Unreachable code.
// Warning 6133: (103-107='test'): Statement has no effect.
