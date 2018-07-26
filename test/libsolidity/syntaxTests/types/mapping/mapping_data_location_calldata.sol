contract c {
    mapping(uint => uint) y;
    function f() view public {
        mapping(uint => uint) calldata x = y;
        x;
    }
}
// ----
// TypeError: (81-113): Location has to be storage for mapping.
