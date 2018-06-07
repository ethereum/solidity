contract test {
    function f() returns(uint d) { return 0 ** 0; }
}
// ----
// f()
// -> 1
