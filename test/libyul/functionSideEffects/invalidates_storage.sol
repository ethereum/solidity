{
    function a() { sstore(0, 1) }
    function f() { a() }
    function g() { pop(callcode(100, 0x010, 10, 0x00, 32, 0x0100, 32))}
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a: invalidatesStorage
// f: invalidatesStorage
// g: invalidatesState, invalidatesStorage, invalidatesMemory
