contract C {
    // Check that visibility is also enforced for the fallback function.
    function() {}
}
// ----
// TypeError: (90-103): Fallback function must be defined as "external".
