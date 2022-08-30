contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f0() public {}
    function f2(uint, uint) public {}

    function h() public view {
        abi.encodeCall(this.f0, true ? g0() : g0());
        abi.encodeCall(this.f2, true ? g2() : g2());
        abi.encodeCall(this.f2, true ? (1, 2) : (3, 4));
    }
}
// ----
// TypeError 9062: (251-269): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (304-322): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (357-379): Expected an inline tuple, not an expression of a tuple type.
