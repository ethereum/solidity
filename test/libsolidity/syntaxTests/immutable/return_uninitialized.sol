contract C {
    uint immutable x;
    constructor() public {
        return;

        x = 1;
    }
}
// ----
// TypeError 2658: (70-77): Construction control flow ends without initializing all immutable state variables.
