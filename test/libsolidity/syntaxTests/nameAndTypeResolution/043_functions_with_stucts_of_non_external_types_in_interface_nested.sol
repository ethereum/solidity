pragma experimental ABIEncoderV2;

contract C {
    struct T { mapping(uint => uint) a; }
    struct S { T[][2] b; }
    function f(S memory) public {}
}
// ----
// TypeError: (132-140): Types containing (nested) mappings can only be used in storage.
// TypeError: (132-140): Only libraries are allowed to use a (nested) mapping type in public or external functions.
