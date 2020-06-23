contract X {}
contract D {
    constructor() X(5) {}
}
// ----
// TypeError 4659: (45-49): Referenced declaration is neither modifier nor base class.
