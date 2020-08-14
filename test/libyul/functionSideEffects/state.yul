{
    function a() { pop(call(100, 0x010, 10, 0x00, 32, 0x0100, 32))}
    function f() { a() }
    function g() { sstore(0, 1) }
}
// ----
// : movable, sideEffectFree, sideEffectFreeIfNoMSize
// a: invalidatesStorage, invalidatesMemory
// f: invalidatesStorage, invalidatesMemory
// g: invalidatesStorage
