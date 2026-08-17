pragma abicoder               v2;

contract C {
    uint[][2] public tmp_i;
    function i(uint[][2] calldata s) external { tmp_i = s; }
}
// ====
// compileViaYul: true
// ----
// i(uint256[][2]): 0x20, 0x40, 0xC0, 3, 0x0A01, 0x0A02, 0x0A03, 4, 0x0B01, 0x0B02, 0x0B03, 0x0B04
// gas irOptimized: 223097
// gas ssaCFGOptimized: 223100
// tmp_i(uint256,uint256): 0, 0 -> 0x0A01
// tmp_i(uint256,uint256): 1, 0 -> 0x0B01
