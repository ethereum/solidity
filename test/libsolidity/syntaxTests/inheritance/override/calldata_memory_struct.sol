pragma experimental ABIEncoderV2;
contract A {
    uint dummy;
    struct S { int a; }
    function f(S calldata) external pure {}
    function g(S calldata) external view { dummy; }
    function h(S calldata) external { dummy = 42; }
    function i(S calldata) external payable {}
}
contract B is A {
    function f(S memory) public pure {}
    function g(S memory) public view { dummy; }
    function h(S memory) public { dummy = 42; }
    function i(S memory) public payable {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
