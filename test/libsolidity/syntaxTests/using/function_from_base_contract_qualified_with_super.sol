contract C {
    function baseFunction(uint) public pure {}
}

contract D is C {
    using {super.baseFunction} for uint;
}
// ----
// DeclarationError 9589: (92-110): Identifier is not a function name or not unique.
