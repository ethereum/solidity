pragma abicoder               v2;
contract C {
    struct S { uint256 x; }
    function f(S calldata s) external pure {
        s.x = 42;
    }
}
// ----
// TypeError 4156: (128-131): Calldata structs are read-only.
