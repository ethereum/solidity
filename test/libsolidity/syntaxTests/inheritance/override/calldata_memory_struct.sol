pragma experimental ABIEncoderV2;
contract A {
    uint dummy;
    struct S { int a; }
    function f(S calldata) external virtual pure {}
    function g(S calldata) external virtual view { dummy; }
    function h(S calldata) external virtual { dummy = 42; }
    function i(S calldata) external virtual payable {}
}
contract B is A {
    function f(S memory) public override pure {}
    function g(S memory) public override view { dummy; }
    function h(S memory) public override { dummy = 42; }
    function i(S memory) public override payable {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
