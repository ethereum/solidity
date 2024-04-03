contract C {
    function g1() internal pure returns (uint) { return (1); }

    error e1(uint);

    function h() public pure {
        uint a;

        abi.encodeError(e1, (a) = g1());
        abi.encodeError(e1, (a) = (1));
    }
}
// ----
