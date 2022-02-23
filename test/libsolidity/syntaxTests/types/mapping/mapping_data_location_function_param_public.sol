contract c {
    function f3(mapping(uint => uint) memory) view public {}
}
// ----
// TypeError 4103: (29-57): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
