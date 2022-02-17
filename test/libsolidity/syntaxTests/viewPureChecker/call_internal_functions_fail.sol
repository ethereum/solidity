contract C {
    uint x;
    function f() pure public { g(); }
    function g() view public { x; }
    function h() view public { i(); }
    function i() public { x = 2; }
}
// ----
// TypeError 2527: (56-59): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (130-133): Function cannot be declared as view because this expression (potentially) modifies the state.
