contract C {
    uint immutable singleBranchWrite; // good

    constructor() {
        if (true)
            singleBranchWrite = 1;
        else
            singleBranchWrite = 1;
    }
}
// ----
