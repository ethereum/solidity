contract C {
    uint x;
    function f() pure public { g(); }
    function g() view public { x; }
    function h() view public { i(); }
    function i() public { x = 2; }
}
// ----
// TypeError: (56-59): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (130-133): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
