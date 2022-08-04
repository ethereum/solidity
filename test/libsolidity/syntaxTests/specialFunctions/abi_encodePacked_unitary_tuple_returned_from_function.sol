contract C {
    function g1() internal pure returns (uint) { return (1); }

    function h() public pure {
        abi.encodePacked(g1());
    }
}
// ----
