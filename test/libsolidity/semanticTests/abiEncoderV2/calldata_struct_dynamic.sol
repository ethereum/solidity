pragma experimental ABIEncoderV2;

contract C {
    struct S { uint256[] a; }

    function f(S calldata s) external returns (bytes memory) {
        return abi.encode(s);
    }

    function g(S calldata s) external returns (bytes memory) {
        return this.f(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f((uint256[])): 0x20, 0x20, 3, 42, 23, 17 -> 32, 192, 0x20, 0x20, 3, 42, 23, 17
// g((uint256[])): 0x20, 0x20, 3, 42, 23, 17 -> 32, 192, 0x20, 0x20, 3, 42, 23, 17
