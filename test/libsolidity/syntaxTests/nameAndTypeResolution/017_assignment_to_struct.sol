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
