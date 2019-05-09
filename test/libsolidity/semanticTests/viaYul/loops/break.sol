contract C {
    function f() public returns (uint x) {
        x = 1;
        for (uint a = 0; a < 10; a = a + 1) {
            x = x + x;
            break;
        }
    }
    function g() public returns (uint x) {
        x = 1;
        uint a = 0;
        while (a < 10) {
            x = x + x;
            break;
            a = a + 1;
        }
    }
    function h() public returns (uint x) {
        x = 1;
        do {
            x = x + 1;
            break;
        } while (x < 3);
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 2
// g() -> 2
// h() -> 2
