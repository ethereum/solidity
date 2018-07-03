contract C { constructor(uint a) public {} }
contract B is C {
    constructor() C(2) C(2) public {}
}
// ----
// DeclarationError: (81-85): Base constructor arguments given twice.
// DeclarationError: (86-90): Base constructor already provided.
