function id(int8 x) pure returns(int8) {
    return x;
}
function id(uint256 x) pure returns(uint256) {
    return x;
}

contract C {
    using {id} for uint256;
}
// ----
// DeclarationError 7920: (145-147): Identifier not found or not unique.
