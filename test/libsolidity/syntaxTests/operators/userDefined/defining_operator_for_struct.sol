using {add as +} for S global;

struct S {
    uint x;
}

function add(S memory, S memory) pure returns (S memory) {}
// ----
// TypeError 5332: (7-10): Operators can only be implemented for user-defined value types.
