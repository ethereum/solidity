contract C {
    uint immutable nestedNonAssignInitInBranch;

    constructor() {
        if (true) {
            nestedNonAssignInitInBranch = 1;
        } else {
            if (true) {
                nestedNonAssignInitInBranch = 1;
            } else {
                delete nestedNonAssignInitInBranch; // init error
            }
        }
    }
}
// ----
// TypeError 3969: (281-308): Immutable variables must be initialized using an assignment.
