contract c {
    function f1(mapping(uint => uint) calldata) pure external returns (mapping(uint => uint) memory) {}
}
// ----
// TypeError: (29-59): Mapping types can only have a data location of "storage" and thus only be parameters or return variables for internal or library functions.
// TypeError: (84-112): Mapping types can only have a data location of "storage" and thus only be parameters or return variables for internal or library functions.
