pragma abicoder               v2;

contract C {
  uint256[] s;
  function f(uint256[] calldata data) external returns (uint) {
    s = data;
    return s[0];
  }
}
// ----
// f(uint256[]): 0x20, 0x03, 0x1, 0x2, 0x3 -> 0x1
// gas irOptimized: 110970
// gas legacy: 112436
// gas legacyOptimized: 111313
// gas ssaCFGOptimized: 110963
