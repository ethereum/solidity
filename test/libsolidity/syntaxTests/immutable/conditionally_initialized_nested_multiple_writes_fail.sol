contract C {
    uint immutable nestedMultipleWrites;

    constructor() {
        if (true) {
            if (true) {
                nestedMultipleWrites = 1;
            } else {
                nestedMultipleWrites = 1;
            }
            nestedMultipleWrites = 2; // error
        } else {
            nestedMultipleWrites = 1;
        }
    }
}
// ----
// TypeError 1574: (250-270): Immutable state variable already initialized.
