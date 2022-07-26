struct S { uint v; }

using {add as +} for S;

function add(S memory _a, S memory) pure returns (S memory) {
    return _a;
}

function g() pure returns (S memory) {
    S memory a = S(0);
    S memory b = S(1);
    S memory c = S(2);
    return a + b + c;
}
