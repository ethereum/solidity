type Int is int16;

using {revert as +} for Int global;
// ----
// DeclarationError 9589: (27-33): Identifier is not a function name or not unique.
