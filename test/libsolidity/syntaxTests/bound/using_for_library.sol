library L {}
library M {}
contract C {
    using L for M;
    using M for L;
    using L for L;
}
// ----
// TypeError 1130: (55-56='M'): Invalid use of a library name.
// TypeError 1130: (74-75='L'): Invalid use of a library name.
// TypeError 1130: (93-94='L'): Invalid use of a library name.
