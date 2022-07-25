contract C {
    function g0() internal pure {}
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    function f0() public {}
    function f2(uint, uint) public {}

    function h() public view {
        abi.encodeCall(this.f0, g0());
        abi.encodeCall(this.f2, g2());

        abi.encodeCall(this.f0, (g0()));
        abi.encodeCall(this.f2, (g2()));
    }
}
// ----
// TypeError 9062: (251-255): Expected an inline tuple, not an expression of a tuple type.
// TypeError 9062: (290-294): Expected an inline tuple, not an expression of a tuple type.
// TypeError 6473: (331-335): Tuple component cannot be empty.
// TypeError 7788: (306-337): Expected 0 instead of 1 components for the tuple parameter.
// TypeError 7788: (347-378): Expected 2 instead of 1 components for the tuple parameter.
// TypeError 5407: (372-376): Cannot implicitly convert component at position 0 from "tuple(uint256,uint256)" to "uint256".
