contract C {
    event E(uint x);

    uint a = 1000 E;
}
// ----
// TypeError 4438: (53-54): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 7407: (48-54): Type tuple() is not implicitly convertible to expected type uint256.
