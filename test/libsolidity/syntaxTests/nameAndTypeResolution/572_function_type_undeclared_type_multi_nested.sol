contract C {
    function a(function(function(function(Nested)))) external pure {}
}
// ----
// DeclarationError: (55-61): Identifier not found or not unique.
