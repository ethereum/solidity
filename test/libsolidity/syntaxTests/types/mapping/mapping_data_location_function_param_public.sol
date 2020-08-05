contract c {
    function f3(mapping(uint => uint) memory) view public {}
}
// ----
// TypeError 4103: (29-57): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
// TypeError 4061: (29-57): Type mapping(uint256 => uint256) is only valid in storage because it contains a (nested) mapping.
