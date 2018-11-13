contract test {
    mapping(uint=>uint) map;
    function fun() public {
        mapping(uint=>uint) storage a = map;
        map = a;
        (map) = a;
        (map, map) = (a, a);
    }
}
// ----
// TypeError: (126-129): Mappings cannot be assigned to.
// TypeError: (144-147): Mappings cannot be assigned to.
// TypeError: (163-166): Mappings cannot be assigned to.
// TypeError: (168-171): Mappings cannot be assigned to.
