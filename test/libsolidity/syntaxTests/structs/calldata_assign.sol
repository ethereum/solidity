pragma experimental ABIEncoderV2;
contract Test {
    struct S { int a; }
    function f(S calldata s) external { s.a = 4; }
}
// ----
// TypeError: (114-117): Calldata structs are read-only.
