error E();

contract C {
    E x;
}
// ----
// TypeError 5172: (29-30): Name has to refer to a user-defined value type, struct, enum or contract.
