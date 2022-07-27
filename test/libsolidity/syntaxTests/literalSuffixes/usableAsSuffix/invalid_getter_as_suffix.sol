contract C {
    uint public g;

    uint a = 1 this.g;
}
// ----
// TypeError 4438: (46-54): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
