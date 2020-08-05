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
// TypeError 9214: (126-129): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (144-147): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (163-166): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (168-171): Types in storage containing (nested) mappings cannot be assigned to.
