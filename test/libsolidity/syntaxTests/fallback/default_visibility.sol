contract C {
    // Check that visibility is also enforced for the fallback function.
    function() {}
}
// ----
// Warning: (90-103): No visibility specified. Defaulting to "public". 
