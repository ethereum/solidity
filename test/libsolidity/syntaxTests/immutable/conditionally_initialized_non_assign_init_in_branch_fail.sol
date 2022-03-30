contract C {
    uint immutable nonAssignInitInBranch;

    constructor() {
        if (true) {
            nonAssignInitInBranch = 1;
        } else {
            delete nonAssignInitInBranch; // init error
        }
    }
}
// ----
// TypeError 3969: (171-192): Immutable variables must be initialized using an assignment.
