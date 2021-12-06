pragma abicoder               v2;

contract Test {
    function f(mapping(uint => uint)[] memory x) public pure {}
}
// ----
// TypeError 4103: (66-98): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
