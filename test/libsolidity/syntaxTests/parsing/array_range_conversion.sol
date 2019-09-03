contract C {
    function f() public pure {
        uint[] memory x;
        uint[1:](x);
        uint[1:2](x);
        uint[][1:](x);
    }
}
// ----
// TypeError: (77-85): Types cannot be sliced.
// TypeError: (77-88): Explicit type conversion not allowed from "uint256[] memory" to "uint256".
// TypeError: (98-107): Types cannot be sliced.
// TypeError: (98-110): Explicit type conversion not allowed from "uint256[] memory" to "uint256".
// TypeError: (120-130): Types cannot be sliced.
