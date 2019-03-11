contract c {
    function f() public pure {
        uint a = -1;
    }
}
// ----
// TypeError: (52-63): Type int_const -1 is not implicitly convertible to expected type uint256. Cannot implicitly convert signed literal to unsigned type.
