contract C {
    function f() public returns (uint x) {
        x = 1;
        for (uint a = 0; a < 10; a = a + 1) {
            x = x + x;
        }
    }
    function g() public returns (uint x) {
        x = 1;
        uint a = 0;
        while (a < 10) {
            x = x + x;
            a = a + 1;
        }
    }
    function h() public returns (uint x) {
        x = 1;
        do {
            x = x + 1;
        } while (false);
    }
    function i() public returns (uint x) {
        x = 1;
        do {
            x = x + 1;
        } while (x < 3);
    }
    function j() public {
        for (;;) {break;}
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 1024
// g() -> 1024
// h() -> 2
// i() -> 3
// j() ->
