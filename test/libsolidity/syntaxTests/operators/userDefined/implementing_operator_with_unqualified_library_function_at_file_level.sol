type B32 is bytes32;

library L {
    function publicOperator(B32, B32) public pure returns (B32) {}
}

using {publicOperator as +} for B32 global;
// ----
// DeclarationError 9589: (111-125): Identifier is not a function name or not unique.
