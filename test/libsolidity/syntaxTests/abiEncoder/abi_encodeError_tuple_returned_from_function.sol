contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    error e0();
    error e2(uint, uint);

    function h() public view {
        abi.encodeError(e0, g0());
        abi.encodeError(e2, g2());

        abi.encodeError(e0, (g0()));
        abi.encodeError(e2, (g2()));
    }
}
// ----
// TypeError 9063: (223-227): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9063: (258-262): Expected an inline tuple, not an expression of a tuple type.
// TypeError 6473: (295-299): Tuple component cannot be empty.
// TypeError 7789: (274-301): Expected 0 instead of 1 components for the tuple parameter.
// TypeError 7789: (311-338): Expected 2 instead of 1 components for the tuple parameter.
// TypeError 5408: (332-336): Cannot implicitly convert component at position 0 from "tuple(uint256,uint256)" to "uint256".
