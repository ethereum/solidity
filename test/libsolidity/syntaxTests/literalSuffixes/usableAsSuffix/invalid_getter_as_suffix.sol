contract C {
    uint public g;

    uint a = 1 this.g;
}
// ----
// TypeError 4438: (46-54): The literal suffix must be either a subdenomination or a file-level suffix function.
