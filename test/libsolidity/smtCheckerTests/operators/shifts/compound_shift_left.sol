contract C {
    function f(uint256 a, uint256 b) internal pure returns (uint256) {
        a <<= b;
        return a;
    }
    function t() public pure {
        assert(f(0x4266, 0x0) == 0x4266);
        assert(f(0x4266, 0x8) == 0x426600);
        assert(f(0x4266, 0xf0) == 0x4266000000000000000000000000000000000000000000000000000000000000);
        assert(f(0x4266, 0x4266) == 0);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
