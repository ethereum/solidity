contract C {
    function f() internal {}
    function g() internal {}
}

contract D is C {
    function h(bool b) public pure {
        (b ? C.f : C.g);
    }
}
// ----
