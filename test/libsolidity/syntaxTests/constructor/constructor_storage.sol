contract A {
    constructor(uint[] storage a) {}
}
// ----
// TypeError 3644: (29-45): This parameter has a type that can only be used internally. You can make the contract abstract to avoid this problem.
