contract C {
    function f() public pure returns (mapping(uint=>uint) memory m) {
    }
}
// ----
// TypeError 4103: (51-79): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (51-79): Type mapping(uint256 => uint256) is only valid in storage because it contains a (nested) mapping.
