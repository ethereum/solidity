contract D {
    mapping (uint => uint) a;
    mapping (uint => uint) b;
    function foo() public view {
        mapping (uint => uint) storage c = b;
        b = c;
    }
}
// ----
// TypeError: (160-161): Mappings cannot be assigned to.
