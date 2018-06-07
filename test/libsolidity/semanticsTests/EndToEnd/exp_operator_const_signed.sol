contract test {
    function f() returns(int d) { return (-2) ** 3; }
}
// ----
// f()
// -> -8
