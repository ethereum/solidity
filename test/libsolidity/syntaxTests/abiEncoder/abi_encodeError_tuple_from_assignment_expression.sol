contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    error e0();
    error e2(uint, uint);

    function h() public view {
        uint a;
        uint b;

        abi.encodeError(e0, () = g0());
        abi.encodeError(e0, () = ());
        abi.encodeError(e2, (a, b) = g2());
        abi.encodeError(e2, (a, b) = (2, 3));
    }
}
// ----
// TypeError 5547: (256-258): Empty tuple on the left hand side.
// TypeError 9063: (256-265): Expected an inline tuple, not an expression of a tuple type.
// TypeError 5547: (296-298): Empty tuple on the left hand side.
// TypeError 9063: (296-303): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9063: (334-347): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9063: (378-393): Expected an inline tuple, not an expression of a tuple type.
