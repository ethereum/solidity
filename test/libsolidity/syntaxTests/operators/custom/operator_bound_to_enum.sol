using {add as +} for E;

enum E {
    E1,
    E2
}

function add(E, E) pure returns (E) {
    return E.E1;
}

// ----
// TypeError 9921: (0-23): The "using" directive cannot be used to bind functions to enum types.
