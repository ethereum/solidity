contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        var a = data.map;
        data.map = a;
    }
}
// ----
// Warning: (122-127): Use of the "var" keyword is deprecated.
// TypeError: (148-160): Mappings cannot be assigned to.
