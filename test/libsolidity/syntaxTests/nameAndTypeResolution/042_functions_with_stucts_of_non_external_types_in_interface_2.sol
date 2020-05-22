pragma experimental ABIEncoderV2;

contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// TypeError: (105-113): Types containing (nested) mappings can only be used in storage.
// TypeError: (105-113): Only libraries are allowed to use a (nested) mapping type in public or external functions.
