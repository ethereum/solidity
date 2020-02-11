contract A {
    uint x = 1;
    uint y = 2;

    function a() public returns(uint x) {
        x = A.y;
    }
}

// ----
// a() -> 2
