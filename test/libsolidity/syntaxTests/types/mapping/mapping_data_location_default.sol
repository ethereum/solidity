contract c {
    mapping(uint => uint) y;
    function f() view public {
        mapping(uint => uint) x = y;
        x;
    }
}
// ----
// TypeError: (81-104): Data location for mappings must be specified as "storage".
