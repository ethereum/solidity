contract C {
    struct S { uint x; }
    S s;
    function f() pure internal returns (S storage) {
        return s;
    }
    function g() pure public {
        f().x;
    }
}
// ----
// TypeError 2527: (115-116): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (163-168): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
