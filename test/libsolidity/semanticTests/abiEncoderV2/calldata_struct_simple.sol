pragma experimental ABIEncoderV2;

contract C {
    struct S { uint256 a; }

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
// f((uint256)): 3 -> 32, 32, 3
// g((uint256)): 3 -> 32, 32, 3
