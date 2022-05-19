contract C {
  event ev0(uint[] i0, uint);
  bytes public s2;
  function h() external returns (bytes memory) {
    uint[] memory x = new uint[](2);
    emit ev0(x, 0x21);
    bytes memory m = new bytes(63);
    s2 = m;
    s2.push();
    return s2;
  }
  function g() external returns (bytes memory) {
    bytes memory m = new bytes(63);
    assembly {
      mstore8(add(m, add(32, 63)), 0x42)
    }
    s2 = m;
    s2.push();
    return s2;
  }
  function f(bytes calldata c) external returns (bytes memory) {
    s2 = c;
    s2.push();
    return s2;
  }
}
// ====
// compileViaYul: also
// ----
// constructor() ->
// gas irOptimized: 520903
// gas legacy: 731840
// gas legacyOptimized: 494859
// h() -> 0x20, 0x40, 0x00, 0
// ~ emit ev0(uint256[],uint256): 0x40, 0x21, 0x02, 0x00, 0x00
// g() -> 0x20, 0x40, 0, 0x00
// f(bytes): 0x20, 33, 0, -1 -> 0x20, 0x22, 0, 0xff00000000000000000000000000000000000000000000000000000000000000
