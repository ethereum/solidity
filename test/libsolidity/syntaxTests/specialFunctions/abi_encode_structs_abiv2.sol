pragma experimental ABIEncoderV2;

contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        abi.encode(s, t);
    }
    function g() public view {
        abi.encodePacked(s, t);
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
