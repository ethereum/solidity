pragma experimental ABIEncoderV2;

contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function e() public view {
        S memory st;
        abi.encodePacked(st);
    }
    function f() public view {
        abi.encode(s, t);
    }
    function g() public view {
        abi.encodePacked(s, t);
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (193-195): Type not supported in packed mode.
// TypeError: (323-324): Type not supported in packed mode.
// TypeError: (326-327): Type not supported in packed mode.
