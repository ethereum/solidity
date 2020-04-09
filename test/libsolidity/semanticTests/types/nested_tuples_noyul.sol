contract test {
    function f3() public returns(int) {
        int a = 3;
        ((, ), ) = ((7, 8), 9);
        return a;
    }
    function f4() public returns(int) {
        int a;
        (a, ) = (4, (8, 16, 32));
        return a;
    }
}
// ====
// compileViaYul: false
// ----
// f3() -> 3
// f4() -> 4
