pragma experimental ABIEncoderV2;
contract C {
    struct S { mapping(uint => uint) a; }
    function f(S memory) public {}
}
// ----
// TypeError 4103: (104-112): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (104-112): Type struct C.S is only valid in storage because it contains a (nested) mapping.
