// This tests a crash that occured because we did not stop for fatal errors.
contract C {
    struct S {
        ftring a;
    }
    S public s;
    function s() s {
    }
}
// ----
// DeclarationError: (113-119): Identifier not found or not unique.
