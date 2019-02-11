pragma experimental ABIEncoderV2;
contract Test {
    struct S { int[3] a; }
    function f(S calldata s, int[3] calldata a) external {
        s.a = a;
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (144-147): Expression has to be an lvalue.
