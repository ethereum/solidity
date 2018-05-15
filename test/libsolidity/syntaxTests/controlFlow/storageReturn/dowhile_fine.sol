contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        do {} while((c = s).f);
    }
    function g() internal view returns (S storage c) {
        do { c = s; } while(false);
    }
    function h() internal view returns (S storage c) {
        c = s;
        do {} while(false);
    }
    function i() internal view returns (S storage c) {
        do {} while(false);
        c = s;
    }
    function j() internal view returns (S storage c) {
        do {
            c = s;
            break;
        } while(false);
    }
    function k() internal view returns (S storage c) {
        do {
            c = s;
            continue;
        } while(false);
    }
}
// ----
