contract C {
    event E(uint x);

    uint a = 1000 E;
}
// ----
// TypeError 4438: (48-54): The literal suffix must be either a subdenomination or a file-level suffix function.
