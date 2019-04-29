contract C {
    function f() public returns (uint x) {
        x = 1;
        for (uint a = 0; a < 10; a = a + 1) {
            x = x + x;
        }
    }
    function g() public returns (uint x) {
        x = 1;
        for (uint a = 0; a < 10; a = a + 1) {
            x = x + x;
            break;
        }
    }
    function h() public returns (uint x) {
        x = 1;
        uint a = 0;
        for (; a < 10; a = a + 1) {
            continue;
            x = x + x;
        }
        x = x + a;
    }
    function i() public returns (uint x) {
        x = 1;
        uint a;
        for (; a < 10; a = a + 1) {
            return x;
            x = x + x;
        }
        x = x + a;
    }
}
// ===
// compileViaYul: true
// ----
// f() -> 1024
// g() -> 2
// h() -> 11
// i() -> 1
