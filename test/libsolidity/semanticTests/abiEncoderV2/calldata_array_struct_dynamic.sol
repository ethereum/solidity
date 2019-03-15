pragma experimental ABIEncoderV2;

contract C {
    struct S { uint256[] a; }
    function f(S[] calldata s) external pure returns (bytes memory) {
        return abi.encode(s);
    }
    function g(S[] calldata s) external view returns (bytes memory) {
        return this.f(s);
    }
}
// ====
// EVMVersion: >homestead
// ----
// f((uint256[])[]): 32, 1, 32, 32, 3, 17, 42, 23 -> 32, 256, 32, 1, 32, 32, 3, 17, 42, 23
// g((uint256[])[]): 32, 1, 32, 32, 3, 17, 42, 23 -> 32, 256, 32, 1, 32, 32, 3, 17, 42, 23
