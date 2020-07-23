contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        str storage a = data;
        data = a;
    }
}
// ----
// TypeError 9214: (152-156): Types in storage containing (nested) mappings cannot be assigned to.
