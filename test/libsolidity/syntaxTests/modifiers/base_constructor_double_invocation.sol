contract C { constructor(uint a) {} }
contract B is C {
    constructor() C(2) C(2) {}
}
// ----
// DeclarationError 3364: (74-78): Base constructor arguments given twice.
// DeclarationError 1697: (79-83): Base constructor already provided.
