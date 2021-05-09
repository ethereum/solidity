contract c {
    function f() public pure {
        uint a = -1;
        uint b = uint(-1);
    }
}
// ----
// TypeError 9574: (52-63): Type int_const -1 is not implicitly convertible to expected type uint256. Cannot implicitly convert signed literal to unsigned type.
// TypeError 9640: (82-90): Explicit type conversion not allowed from "int_const -1" to "uint256".
