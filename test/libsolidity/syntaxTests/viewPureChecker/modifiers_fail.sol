contract D {
    uint x;
    modifier viewm(uint) { uint a = x; _; a; }
    modifier nonpayablem(uint) { x = 2; _; }
}
contract C is D {
    function f() viewm(0) pure public {}
    function g() nonpayablem(0) view public {}
}
// ----
// TypeError: (154-162): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (195-209): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
