contract D { }
contract C {
    using D for uint;
}
// ----
// TypeError 4357: (38-39): Library name expected.
