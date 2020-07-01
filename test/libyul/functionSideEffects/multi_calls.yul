{
    function a() {
        b()
    }
    function b() {
        sstore(0, 1)
        b()
    }
    function c() {
        mstore(0, 1)
        a()
        d()
    }
    function d() {
    }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: can loop, writes storage
// b: can loop, writes storage
// c: can loop, writes storage, writes memory
// d: movable, movable apart from effects, can be removed, can be removed if no msize
