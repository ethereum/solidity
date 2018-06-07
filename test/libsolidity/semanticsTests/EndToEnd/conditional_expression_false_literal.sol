contract test {
    function f() returns(uint d) {
        return false ? 5 : 10;
    }
}
// ----
// f()
// -> 10
