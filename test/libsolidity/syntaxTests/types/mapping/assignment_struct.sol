contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        mapping(uint=>uint) storage a = data.map;
        data.map = a;
        (data.map) = a;
        (data.map, data.map) = (a, a);
    }
}
// ----
// TypeError 9214: (172-180='data.map'): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (195-203='data.map'): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (219-227='data.map'): Types in storage containing (nested) mappings cannot be assigned to.
// TypeError 9214: (229-237='data.map'): Types in storage containing (nested) mappings cannot be assigned to.
