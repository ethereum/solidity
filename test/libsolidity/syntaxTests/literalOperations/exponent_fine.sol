contract C {
    function f() public pure {
        uint a;
        a = a ** 1E5;
        a = 0 ** 1E1233;
    }
}
// ----
