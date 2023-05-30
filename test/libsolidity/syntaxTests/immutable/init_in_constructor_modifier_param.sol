contract D {
    uint immutable t;
    modifier m(uint) { _; }
    constructor() m(t = 2) {}
}
// ----
