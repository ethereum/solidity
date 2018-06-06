contract C {
    function f() pure public { g(); }
    function g() view public {}
}
// ----
// TypeError: (44-47): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// Warning: (55-82): Function state mutability can be restricted to pure
