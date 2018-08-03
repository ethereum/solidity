contract test {
    mapping(uint=>uint) map;
    function fun() public view {
        mapping(uint=>uint) storage a = map;
        mapping(uint=>uint) storage b = map;
        b = a;
        (b) = a;
        (b, b) = (a, a);
    }
}
// ----
// TypeError: (176-177): Mappings cannot be assigned to.
// TypeError: (192-193): Mappings cannot be assigned to.
// TypeError: (209-210): Mappings cannot be assigned to.
// TypeError: (212-213): Mappings cannot be assigned to.
