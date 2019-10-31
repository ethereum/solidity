pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// TypeError: (104-112): Type is required to live outside storage.
// TypeError: (104-112): Only libraries are allowed to use the mapping type in public or external functions.
