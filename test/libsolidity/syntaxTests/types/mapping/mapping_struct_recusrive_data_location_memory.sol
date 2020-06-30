pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    struct T { S s; }
    struct U { T t; }
    function f(U memory) public {}
}
// ----
// TypeError 4103: (148-156): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (148-156): Type struct C.U is only valid in storage because it contains a (nested) mapping.
