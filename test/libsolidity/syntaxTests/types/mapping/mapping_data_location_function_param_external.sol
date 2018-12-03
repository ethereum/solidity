contract c {
    function f1(mapping(uint => uint) calldata) pure external returns (mapping(uint => uint) memory) {}
}
// ----
// TypeError: (29-59): Mapping types for parameters or return variables can only be used in internal or library functions.
// TypeError: (84-112): Mapping types for parameters or return variables can only be used in internal or library functions.
