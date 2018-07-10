contract C {
    // Check that visibility is also enforced for the fallback function.
    function() {}
}
// ----
// SyntaxError: (90-103): No visibility specified. Did you intend to add "external"?
// TypeError: (90-103): Fallback function must be defined as "external".
