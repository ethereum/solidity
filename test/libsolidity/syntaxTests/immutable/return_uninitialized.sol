contract C {
    uint immutable x;
    constructor() {
        return;

        x = 1;
    }
}
// ----
// TypeError 2658: (63-70): Construction control flow ends without initializing all immutable state variables.
