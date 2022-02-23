contract c {
    function f1(mapping(uint => uint) calldata) pure external returns (mapping(uint => uint) memory) {}
}
// ----
// TypeError 4103: (29-59): Types containing (nested) mappings can only be parameters or return variables of internal or library functions.
