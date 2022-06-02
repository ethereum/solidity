contract C {
    event E(uint x);

    uint a = 1000 E;

}
// ----
// TypeError 4438: (48-54): The literal suffix needs to be a pre-defined suffix or a file-level function.
