pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    struct T { S s; }
    struct U { T t; }
    function f(U memory) public {}
}
// ----
// TypeError: (148-156): Type is required to live outside storage.
// TypeError: (148-156): Only libraries are allowed to use the mapping type in public or external functions.
