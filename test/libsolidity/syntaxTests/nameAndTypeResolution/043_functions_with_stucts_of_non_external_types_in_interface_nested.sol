pragma experimental ABIEncoderV2;

contract C {
    struct T { mapping(uint => uint) a; }
    struct S { T[][2] b; }
    function f(S memory) public {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (132-140): Only libraries are allowed to use the mapping type in public or external functions.
