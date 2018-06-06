contract c {
    function c () public {
        a = 115792089237316195423570985008687907853269984665640564039458;
    }
    uint256 a;
}
// ----
// Warning: (17-119): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
