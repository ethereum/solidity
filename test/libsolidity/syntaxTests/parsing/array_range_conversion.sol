contract C {
    function f() public pure {
        uint[] memory x;
        uint[1:](x);
        uint[1:2](x);
        uint[][1:](x);
    }
}
// ----
// TypeError 1760: (77-85='uint[1:]'): Types cannot be sliced.
// TypeError 9640: (77-88='uint[1:](x)'): Explicit type conversion not allowed from "uint256[] memory" to "uint256".
// TypeError 1760: (98-107='uint[1:2]'): Types cannot be sliced.
// TypeError 9640: (98-110='uint[1:2](x)'): Explicit type conversion not allowed from "uint256[] memory" to "uint256".
// TypeError 1760: (120-130='uint[][1:]'): Types cannot be sliced.
