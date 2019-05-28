contract C {
    function f() public returns (uint x) {
        x = 1;
        uint a = 0;
        for (; a < 10; a = a + 1) {
            continue;
            x = x + x;
        }
        x = x + a;
    }
    function g() public returns (uint x) {
        x = 1;
        uint a = 0;
        while (a < 10) {
            a = a + 1;
            continue;
            x = x + x;
        }
        x = x + a;
    }
    function h() public returns (uint x) {
        x = 1;
        uint a = 0;
        do {
            a = a + 1;
            continue;
            x = x + x;
        } while (a < 4);
        x = x + a;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 11
// g() -> 11
// h() -> 5
