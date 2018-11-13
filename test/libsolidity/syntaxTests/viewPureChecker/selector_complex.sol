contract C {
    function f(C c) pure public returns (C) {
        return c;
    }
    function g() pure public returns (bytes4) {
        // By passing `this`, we read from the state, even if f itself is pure.
        return f(this).f.selector;
    }
}
// ----
// TypeError: (228-232): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
