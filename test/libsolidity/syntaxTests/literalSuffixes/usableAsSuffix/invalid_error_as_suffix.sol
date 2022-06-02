error E(uint x);

contract C {
    uint a = 1000 E;

}
// ----
// TypeError 4438: (44-50): The literal suffix needs to be a pre-defined suffix or a file-level function.
