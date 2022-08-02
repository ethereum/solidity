using {add as +} for E;

error E();

function add(E, E) pure returns (E) {
    return E.E1;
}

// ----
// TypeError 5172: (21-22): Name has to refer to a user-defined value type, struct, enum or contract.
