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
// DeclarationError 2333: (150-179='function s() public s {     }'): Identifier already declared.
// DeclarationError 7920: (114-120='ftring'): Identifier not found or not unique.
