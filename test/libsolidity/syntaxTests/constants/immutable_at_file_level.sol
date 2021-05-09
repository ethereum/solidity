uint immutable x = 7;
// ----
// DeclarationError 8342: (0-20): Only constant variables are allowed at file level.
// DeclarationError 8297: (0-20): The "immutable" keyword can only be used for state variables.
