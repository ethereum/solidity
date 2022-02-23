contract C {
    mapping(uint => uint) m;
    function f() public {
        m[] = 3;
    }
}
// ----
// TypeError 1267: (76-79): Index expression cannot be omitted.
