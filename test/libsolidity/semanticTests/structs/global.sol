pragma experimental ABIEncoderV2;

struct S { uint256 a; uint256 b; }
contract C {
    function f(S calldata s) external pure returns (uint256, uint256) {
        return (s.a, s.b);
    }
}
// ----
// f((uint256,uint256)): 42, 23 -> 42, 23
