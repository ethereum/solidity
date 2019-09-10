pragma experimental ABIEncoderV2;

contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (105-113): Only libraries are allowed to use the mapping type in public or external functions.
