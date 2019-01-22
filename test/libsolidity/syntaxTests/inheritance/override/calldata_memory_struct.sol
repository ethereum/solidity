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
// TypeError: (102-112): Calldata structs are not yet supported.
// TypeError: (146-156): Calldata structs are not yet supported.
// TypeError: (198-208): Calldata structs are not yet supported.
// TypeError: (250-260): Calldata structs are not yet supported.
