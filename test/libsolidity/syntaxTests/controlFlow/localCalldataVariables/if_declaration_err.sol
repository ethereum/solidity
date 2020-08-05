contract C {
    function f(uint[] calldata _c) public pure {
        uint[] calldata c;
        if (_c[2] > 10)
            c = _c;
        c[2];
    }
}
// ----
// TypeError 3464: (141-142): This variable is of calldata pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
