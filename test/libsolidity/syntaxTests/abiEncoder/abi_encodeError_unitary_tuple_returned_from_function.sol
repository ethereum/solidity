contract C {
    function g1() internal pure returns (uint) { return (1); }
    function g2() internal pure returns (uint, uint) { return (2, 3); }

    error e1(uint);
    error e2(uint, uint);

    function h() public pure {
        abi.encodeError(e1, g1());
        abi.encodeError(e1, (g1()));
        abi.encodeError(e2, (g1(), g1()));
    }
}
// ----
