using {add as +} for S;

struct S {
    uint x;
}

function add(S memory, S memory) pure returns (S memory) {}
