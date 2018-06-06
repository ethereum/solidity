contract c {
    function c () public {
        a = 115792089237316195423570985008687907853269984665640564039458 ether;
    }
    uint256 a;
}
// ----
// Warning: (17-125): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (52-118): Type int_const 1157...(70 digits omitted)...0000 is not implicitly convertible to expected type uint256.
