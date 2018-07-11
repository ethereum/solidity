// This tests a crash that occurred because we did not stop for fatal errors.
contract C {
    struct S {
        ftring a;
    }
    S public s;
    function s() public s {
    }
}
// ----
// DeclarationError: (114-120): Identifier not found or not unique.
