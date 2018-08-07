contract c {
    mapping(uint => uint) y;
    function f() view public {
        mapping(uint => uint) x = y;
        x;
    }
}
// ----
// TypeError: (81-104): Location has to be storage for mapping. Use an explicit data location keyword to fix this error.
