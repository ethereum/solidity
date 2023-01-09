contract C {
    using {this.contractFunction} for uint;

    function contractFunction(uint) external view {}
}
// ----
// DeclarationError 9589: (24-45): Identifier is not a function name or not unique.
