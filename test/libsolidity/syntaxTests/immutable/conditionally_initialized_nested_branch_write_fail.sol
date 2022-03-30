contract C {
    uint immutable nestedBranchWrite; // error

    constructor() {
        if (true) {
            nestedBranchWrite = 1;
        } else {
            if (true)
                nestedBranchWrite = 2;
        }
    }
}
// ----
// TypeError 4599: (32-49): Immutable is not initialized in every branch of control flow.
