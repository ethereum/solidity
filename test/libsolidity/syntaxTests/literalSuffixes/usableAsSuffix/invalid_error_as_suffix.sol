error E(uint x);

contract C {
    uint a = 1000 E;
}
// ----
// TypeError 4438: (44-50): The literal suffix must be either a subdenomination or a file-level suffix function.
