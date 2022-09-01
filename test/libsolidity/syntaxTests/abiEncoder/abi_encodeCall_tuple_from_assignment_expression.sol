contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f0() public {}
    function f2(uint, uint) public {}

    function h() public view {
        uint a;
        uint b;

        abi.encodeCall(this.f0, () = g0());
        abi.encodeCall(this.f0, () = ());
        abi.encodeCall(this.f2, (a, b) = g2());
        abi.encodeCall(this.f2, (a, b) = (2, 3));
    }
}
// ----
// TypeError 5547: (284-286): Empty tuple on the left hand side.
// TypeError 9062: (284-293): Expected an inline tuple, not an expression of a tuple type.
// TypeError 5547: (328-330): Empty tuple on the left hand side.
// TypeError 9062: (328-335): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (370-383): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (418-433): Expected an inline tuple, not an expression of a tuple type.
