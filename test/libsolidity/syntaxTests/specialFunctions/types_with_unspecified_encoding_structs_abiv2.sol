pragma experimental ABIEncoderV2;

contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        bytes32 a = sha256(s, t);
        a;
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (167-179): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// TypeError: (174-175): This type cannot be encoded.
// TypeError: (177-178): This type cannot be encoded.
