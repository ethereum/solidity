contract A {
    constructor(mapping(uint => uint) memory a) {}
}
// ----
// TypeError 4103: (29-59): Types containing (nested) mappings can only be parameters or return variables of internal or library functions. You can make the contract abstract to avoid this problem.
