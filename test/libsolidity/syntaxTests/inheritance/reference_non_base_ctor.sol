contract X {}
contract D {
    constructor() X(5) public {}
}
// ----
// TypeError: (45-49): Referenced declaration is neither modifier nor base class.
