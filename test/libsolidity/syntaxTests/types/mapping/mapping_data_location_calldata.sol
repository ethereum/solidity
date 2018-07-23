contract c {
    mapping(uint => uint) y;
    function f() view public {
        mapping(uint => uint) calldata x = y;
        x;
    }
}
// ----
// TypeError: (81-113): Data location for mappings must be specified as "storage".
