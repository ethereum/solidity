pragma abicoder               v2;

contract C {
    struct T { mapping(uint => uint) a; }
    struct S { T[][2] b; }
    function f(S memory) public {}
}
// ----
// TypeError 4103: (132-140): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
