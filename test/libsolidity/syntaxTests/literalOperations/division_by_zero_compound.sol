contract A {
    uint a = 5;
    constructor() { a /= uint(0); }
}
// ----
