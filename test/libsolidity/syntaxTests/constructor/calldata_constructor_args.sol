contract C {
    constructor(uint[] calldata) public {}
}
// ----
// TypeError 6651: (29-44='uint[] calldata'): Data location must be "storage" or "memory" for constructor parameter, but "calldata" was given.
