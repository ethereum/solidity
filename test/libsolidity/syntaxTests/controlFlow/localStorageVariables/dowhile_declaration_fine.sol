contract C {
    struct S { bool f; }
    S s;
    function f() internal view {
        S storage c;
        do {} while((c = s).f);
        c;
    }
    function g() internal view {
        S storage c;
        do { c = s; } while(false);
        c;
    }
    function h() internal view {
        S storage c;
        c = s;
        do {} while(false);
        c;
    }
    function i() internal view {
        S storage c;
        do {} while(false);
        c = s;
        c;
    }
    function j() internal view {
        S storage c;
        do {
            c = s;
            break;
        } while(false);
        c;
    }
    function k() internal view {
        S storage c;
        do {
            c = s;
            continue;
        } while(false);
        c;
    }
}
// ----
// Warning: (606-611): Unreachable code.
