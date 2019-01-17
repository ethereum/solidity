pragma experimental ABIEncoderV2;

contract C {
    function f(Data.S memory a) public {}
}
contract Data {
    struct S { S x; }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (63-78): Internal or recursive type is not allowed for public or external functions.
