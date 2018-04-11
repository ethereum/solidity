contract C {
    // This is an error, you should explicitly use
    // `external public` to fix it - `internal public` does not exist.
    function(bytes memory) public a;
}
// ----
// TypeError: (139-170): Invalid visibility, can only be "external" or "internal".
