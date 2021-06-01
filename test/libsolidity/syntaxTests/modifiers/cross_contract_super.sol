contract C {
    modifier m() { _; }
}
contract D is C {
    function f() super.m public {
    }
}
// ----
// DeclarationError 7920: (74-81): Identifier not found or not unique.
