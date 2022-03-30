contract C {
    uint immutable nestedBranchWrite; // good

    constructor() {
        if (true) {
            nestedBranchWrite = 1;
        } else {
            if (true)
                nestedBranchWrite = 2;
            else
                nestedBranchWrite = 3;
        }
    }
}
// ----
