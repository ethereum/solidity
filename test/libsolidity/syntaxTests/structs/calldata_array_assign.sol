pragma experimental ABIEncoderV2;
contract Test {
    struct S { int[3] a; }
    function f(S calldata s, int[3] calldata a) external {
        s.a = a;
    }
}
// ----
// TypeError 4156: (144-147): Calldata structs are read-only.
