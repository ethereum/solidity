contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        mapping(uint=>uint) storage a = data.map;
        data.map = a;
    }
}
// ----
// TypeError: (172-184): Mappings cannot be assigned to.
