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
// TypeError: (172-180): Mappings cannot be assigned to.
// TypeError: (195-203): Mappings cannot be assigned to.
// TypeError: (219-227): Mappings cannot be assigned to.
// TypeError: (229-237): Mappings cannot be assigned to.
