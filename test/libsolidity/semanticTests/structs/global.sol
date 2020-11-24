pragma abicoder               v2;

struct S { uint256 a; uint256 b; }
contract C {
    function f(S calldata s) external pure returns (uint256, uint256) {
        return (s.a, s.b);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f((uint256,uint256)): 42, 23 -> 42, 23
