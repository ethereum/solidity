contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        mapping(uint=>uint) a = data.map;
        data.map = a;
    }
}
// ----
// TypeError: (164-176): Mappings cannot be assigned to.
