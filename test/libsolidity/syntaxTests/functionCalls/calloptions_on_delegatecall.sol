contract C {
    function foo() pure internal {
        address(10).delegatecall{value: 7, gas: 3}("");
    }
}
// ----
// TypeError 6189: (56-98): Cannot set option "value" for delegatecall.
// Warning 9302: (56-102): Return value of low-level calls not used.
