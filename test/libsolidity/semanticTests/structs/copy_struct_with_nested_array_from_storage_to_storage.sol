contract C {
    struct S {
        uint8[1] x;
        uint8[] y;
    }

    S src;
    S dst;

    constructor() {
        src.x = [3];
        src.y.push(7);
        src.y.push(11);
    }

    function test() public {
        dst = src;

        require(dst.x[0] == 3);
        require(dst.y.length == 2);
        require(dst.y[0] == 7);
        require(dst.y[1] == 11);
    }
}
// ----
// test()
