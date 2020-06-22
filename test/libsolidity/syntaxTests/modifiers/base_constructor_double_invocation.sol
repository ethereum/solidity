contract C { constructor(uint a) public {} }
contract B is C {
    constructor() C(2) C(2) public {}
}
// ----
// DeclarationError 3364: (81-85): Base constructor arguments given twice.
// DeclarationError 1697: (86-90): Base constructor already provided.
