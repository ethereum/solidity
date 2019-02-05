contract c {
    function f3(mapping(uint => uint) memory) view public {}
}
// ----
// TypeError: (29-57): Mapping types can only have a data location of "storage" and thus only be parameters or return variables for internal or library functions.
