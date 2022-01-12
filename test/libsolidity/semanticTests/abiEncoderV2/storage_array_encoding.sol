pragma abicoder               v2;

// tests encoding from storage arrays

contract C {
    uint256[2][] tmp_h;
    function h(uint256[2][] calldata s) external returns (bytes memory) {
        tmp_h = s;
        return abi.encode(tmp_h);
    }
    uint256[2][2] tmp_i;
    function i(uint256[2][2] calldata s) external returns (bytes memory) {
        tmp_i = s;
        return abi.encode(tmp_i);
    }
}
// ====
// EVMVersion: >homestead
// compileViaYul: also
// ----
// h(uint256[2][]): 0x20, 3, 123, 124, 223, 224, 323, 324 -> 32, 256, 0x20, 3, 123, 124, 223, 224, 323, 324
// gas irOptimized: 180324
// gas legacy: 184390
// gas legacyOptimized: 180940
// i(uint256[2][2]): 123, 124, 223, 224 -> 32, 128, 123, 124, 223, 224
// gas irOptimized: 112266
// gas legacy: 115229
// gas legacyOptimized: 112740
