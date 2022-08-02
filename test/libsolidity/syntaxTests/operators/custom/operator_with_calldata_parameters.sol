using {add as +} for S;

struct S {
    uint x;
} 

function add(S calldata, S calldata) pure returns (S calldata r) {
    assembly {
        r := 0
    }
}
