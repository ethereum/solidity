contract C {
    function a(function(Nested)) external pure {}
}
// ----
// DeclarationError 7920: (37-43): Identifier not found or not unique.
