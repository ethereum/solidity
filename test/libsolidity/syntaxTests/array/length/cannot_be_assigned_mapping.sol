contract C {
    mapping(uint => uint[]) map;
    function f() public {
        map[0].length = 4;
    }
}
// ----
// TypeError: (80-93): Member "length" is read-only and cannot be used to resize arrays.
