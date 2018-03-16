contract C { function C(uint a) public {} }
contract B is C {
    function B() C(2) C(2) public {}
}
// ----
// DeclarationError: Base constructor already provided.
