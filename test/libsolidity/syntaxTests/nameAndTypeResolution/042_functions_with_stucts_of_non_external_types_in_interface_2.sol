pragma experimental ABIEncoderV2;

contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (105-106): Internal or recursive type is not allowed for public or external functions.
