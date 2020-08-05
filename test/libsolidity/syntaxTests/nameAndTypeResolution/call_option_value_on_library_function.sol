library L { function l() public {} }
contract test {
    function f() public {
        L.l{value: 1}();
    }
}
// ----
// TypeError 2193: (87-100): Function call options can only be set on external function calls or contract creations.
