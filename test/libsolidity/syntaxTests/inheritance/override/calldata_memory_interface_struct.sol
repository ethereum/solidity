pragma experimental ABIEncoderV2;
interface I {
    struct S { int a; }
    function f(S calldata) external pure;
    function g(S calldata) external view;
    function h(S calldata) external;
    function i(S calldata) external payable;
}
contract C is I {
    uint dummy;
    function f(S memory) public override pure {}
    function g(S memory) public override view { dummy; }
    function h(S memory) public override { dummy = 42; }
    function i(S memory) public override payable {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
