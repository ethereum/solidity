contract C {
    function a(function(function(function(Nested)))) external pure {}
}
// ----
// DeclarationError 7920: (55-61='Nested'): Identifier not found or not unique.
