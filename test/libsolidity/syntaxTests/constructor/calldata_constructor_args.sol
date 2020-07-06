contract C {
    constructor(uint[] calldata) public {}
}
// ----
// TypeError 6651: (29-44): Data location must be "memory" for parameter in function, but "calldata" was given.
