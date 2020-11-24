pragma abicoder               v2;

contract Test {
    function f(mapping(uint => uint)[] memory x) public pure {}
}
// ----
// TypeError 4103: (66-98): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (66-98): Type mapping(uint256 => uint256)[] is only valid in storage because it contains a (nested) mapping.
