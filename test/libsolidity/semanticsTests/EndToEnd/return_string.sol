contract Main {
    string public s;
    function set(string _s) external returns (bool) {
        s = _s;
        return true;
    }
    function get1() returns (string r) {
        return s;
    }
    function get2() returns (string r) {
        r = s;
    }
}
// ----
// set(string): 0x20, 5, "Julia"
// -> true
// get1()
// -> 0x20, 5, "Julia"
// get2()
// -> 0x20, 5, "Julia"
