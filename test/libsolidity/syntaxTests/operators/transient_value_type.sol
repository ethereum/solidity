contract C {
    int transient x;
    function f() public view returns (int) {
        int y = x;
        int w = -x;
        return (x + w) * (y / x);
    }
}
// ====
// stopAfter: parsing
// ----
