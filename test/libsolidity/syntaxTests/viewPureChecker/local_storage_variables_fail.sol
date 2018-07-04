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
// TypeError: (100-101): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (184-187): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
