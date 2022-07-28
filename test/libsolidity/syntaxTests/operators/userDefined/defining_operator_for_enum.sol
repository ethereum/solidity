using {add as +} for E global;

enum E {
    E1,
    E2
}

function add(E, E) pure returns (E) {}
// ----
// TypeError 5332: (7-10): Operators can only be implemented for user-defined value types.
