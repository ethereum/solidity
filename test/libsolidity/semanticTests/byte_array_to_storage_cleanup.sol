contract C {
    event ev(uint[], uint);
    bytes public s;
    function h() external returns (bytes memory) {
        uint[] memory x = new uint[](2);
        emit ev(x, 0x21);
        bytes memory m = new bytes(63);
        s = m;
        s.push();
        return s;
    }
    function g() external returns (bytes memory) {
        bytes memory m = new bytes(63);
        assembly {
            mstore8(add(m, add(32, 63)), 0x42)
        }
        s = m;
        s.push();
        return s;
    }
    function f(bytes calldata c) external returns (bytes memory) {
        s = c;
        s.push();
        return s;
    }
}
// ====
// compileViaYul: also
// ----
// constructor() ->
// gas irOptimized: 442746
// gas legacy: 711299
// gas legacyOptimized: 481080
// h() -> 0x20, 0x40, 0x00, 0
// ~ emit ev(uint256[],uint256): 0x40, 0x21, 0x02, 0x00, 0x00
// g() -> 0x20, 0x40, 0, 0x00
// f(bytes): 0x20, 33, 0, -1 -> 0x20, 0x22, 0, 0xff00000000000000000000000000000000000000000000000000000000000000
