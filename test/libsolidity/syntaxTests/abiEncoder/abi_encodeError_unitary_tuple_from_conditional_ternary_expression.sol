contract C {
    function g1() internal pure returns (uint) { return (1); }

    error e1(uint);

    function h() public pure {
        abi.encodeError(e1, true ? (1) : (2));
        abi.encodeError(e1, true ? g1() : g1());
    }
}
// ----
