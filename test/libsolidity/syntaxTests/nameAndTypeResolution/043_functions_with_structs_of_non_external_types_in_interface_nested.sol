pragma experimental ABIEncoderV2;

contract C {
    struct T { mapping(uint => uint) a; }
    struct S { T[][2] b; }
    function f(S memory) public {}
}
// ----
// TypeError 4103: (132-140): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (132-140): Type struct C.S is only valid in storage because it contains a (nested) mapping.
