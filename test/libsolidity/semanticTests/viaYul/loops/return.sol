contract C {
    function f() public returns (uint x) {
        x = 1;
        uint a;
        for (; a < 10; a = a + 1) {
            return x;
            x = x + x;
        }
        x = x + a;
    }
    function g() public returns (uint x) {
        x = 1;
        uint a;
        while (a < 10) {
            return x;
            x = x + x;
            a = a + 1;
        }
        x = x + a;
    }
    function h() public returns (uint x) {
        x = 1;
        do {
            x = x + 1;
            return x;
        } while (x < 3);
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 1
// g() -> 1
// h() -> 2
