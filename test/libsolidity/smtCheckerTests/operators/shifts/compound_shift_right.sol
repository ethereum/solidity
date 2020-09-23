pragma experimental SMTChecker;

contract C {
    function f(uint256 a, uint256 b) internal pure returns (uint256) {
        a >>= b;
        return a;
    }
    function t() public pure {
        assert(f(0x4266, 0) == 0x4266);
        assert(f(0x4266, 0x8) == 0x42);
        assert(f(0x4266, 0x11) == 0);
        assert(f(57896044618658097711785492504343953926634992332820282019728792003956564819968, 5) == 1809251394333065553493296640760748560207343510400633813116524750123642650624);
    }
}
// ----
