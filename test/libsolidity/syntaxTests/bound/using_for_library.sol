library L {}
library M {}
contract C {
    using L for M;
    using M for L;
    using L for L;
}
// ----
// TypeError 1130: (55-56): Invalid use of a library name.
// TypeError 1130: (74-75): Invalid use of a library name.
// TypeError 1130: (93-94): Invalid use of a library name.
