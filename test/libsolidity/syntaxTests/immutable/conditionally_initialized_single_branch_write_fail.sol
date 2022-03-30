contract C {
    uint immutable singleBranchWrite; // error

    constructor() {
        if (true)
            singleBranchWrite = 1;
    }
}
// ----
// TypeError 4599: (32-49): Immutable is not initialized in every branch of control flow.
