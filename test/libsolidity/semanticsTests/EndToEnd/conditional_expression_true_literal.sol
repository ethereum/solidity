contract test {
    function f() returns(uint d) {
        return true ? 5 : 10;
    }
}
// ----
// f()
// -> 5
