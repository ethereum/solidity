error E(uint x);

contract C {
    uint a = 1000 E;
}
// ----
// TypeError 4438: (49-50): The literal suffix must be either a subdenomination or a file-level suffix function.
// TypeError 7407: (44-50): Type tuple() is not implicitly convertible to expected type uint256.
