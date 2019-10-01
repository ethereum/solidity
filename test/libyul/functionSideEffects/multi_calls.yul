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
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a: invalidatesStorage
// b: invalidatesStorage
// c: invalidatesStorage, invalidatesMemory
// d: movable, sideEffectFree, sideEffectFreeIfNoMSize
