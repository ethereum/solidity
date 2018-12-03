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
    function f(S memory) public pure {}
    function g(S memory) public view { dummy; }
    function h(S memory) public { dummy = 42; }
    function i(S memory) public payable {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
