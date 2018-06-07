contract test {
    function f() returns(uint d) { return 2 ** 3; }
}
// ----
// f()
// -> 8
