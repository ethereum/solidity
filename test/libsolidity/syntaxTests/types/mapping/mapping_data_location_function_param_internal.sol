contract c {
    function f4(mapping(uint => uint) storage) pure internal {}
    function f5(mapping(uint => uint) memory) pure internal {}
}
// ----
// TypeError: (93-121): Mapping types can only have a data location of "storage".
