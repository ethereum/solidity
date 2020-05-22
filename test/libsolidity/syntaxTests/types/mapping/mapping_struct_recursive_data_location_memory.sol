pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    struct T { S s; }
    struct U { T t; }
    function f(U memory) public {}
}
// ----
// TypeError: (148-156): Types containing (nested) mappings can only be used in storage.
// TypeError: (148-156): Only libraries are allowed to use a (nested) mapping type in public or external functions.
