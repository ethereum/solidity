contract C {
    uint immutable nestedMultipleWrites;

    constructor() {
        if (true) {
            if (true) {
                nestedMultipleWrites = 1;
            } else {
                nestedMultipleWrites = 1;
            }
        } else {
            nestedMultipleWrites = 1;
        }
    }
}
// ----
