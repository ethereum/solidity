contract C { constructor(uint) {} }
contract D is C(D.t = 2) {
    uint immutable t;
}
