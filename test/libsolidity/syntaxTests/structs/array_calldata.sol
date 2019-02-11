pragma experimental ABIEncoderV2;
contract Test {
    struct S { int a; }
    function f(S[] calldata) external { }
    function f(S[][] calldata) external { }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (131-145): Calldata arrays with dynamically encoded base types are not yet supported.
