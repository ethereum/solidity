using {add as +} for S;

struct S {
    int v;
}

function add(S storage _a, S storage) returns (S memory) {
    _a.v = 7;
    return _a;
}


contract C {
    S s;
    function f() public returns (S memory) {
        return s + s;
    }
}

// ----
// TypeError 3605: (7-10): The function "add" needs to have parameters and return value of the same type to be used for the operator +.
