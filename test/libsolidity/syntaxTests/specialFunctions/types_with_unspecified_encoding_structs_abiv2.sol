pragma experimental ABIEncoderV2;

contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        bytes32 a = sha256(abi.encodePacked(s, t));
        a;
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (191-192): This type cannot be encoded.
// TypeError: (194-195): This type cannot be encoded.
