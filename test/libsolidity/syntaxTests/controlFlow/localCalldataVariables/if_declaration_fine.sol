contract C {
    function f(uint[] calldata _c) public pure {
        uint[] calldata c;
        if (_c[2] > 10)
            c = _c;
        else
            c = _c;
        c[2];
    }
}
// ----
