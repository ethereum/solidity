contract test {
    function f(int8 x) public pure {
        if (x == 1) {}
        if (1 == x) {}
    }
}
// ----
