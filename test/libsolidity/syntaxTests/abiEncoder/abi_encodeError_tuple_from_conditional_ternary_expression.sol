contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    error e0();
    error e2(uint, uint);

    function h() public view {
        abi.encodeError(e0, true ? g0() : g0());
        abi.encodeError(e2, true ? g2() : g2());
        abi.encodeError(e2, true ? (1, 2) : (3, 4));
    }
}
// ----
// TypeError 9062: (223-241): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (272-290): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (321-343): Expected an inline tuple, not an expression of a tuple type.
