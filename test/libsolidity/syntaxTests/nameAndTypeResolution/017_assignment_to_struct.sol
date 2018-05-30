contract test {
    struct str {
        mapping(uint=>uint) map;
    }
    str data;
    function fun() public {
        var a = data;
        data = a;
    }
}
// ----
// Warning: (122-127): Use of the "var" keyword is deprecated.
