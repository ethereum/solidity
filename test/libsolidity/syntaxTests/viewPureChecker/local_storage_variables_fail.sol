contract C {
    struct S { uint a; }
    S s;
    function f() pure public {
        S storage x = s;
        x;
    }
    function g() view public {
        S storage x = s;
        x.a = 1;
    }
}
// ----
// TypeError 2527: (100-101): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (184-187): Function cannot be declared as view because this expression (potentially) modifies the state.
