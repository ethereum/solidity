{
    function a() { pop(callcode(100, 0x010, 10, 0x00, 32, 0x0100, 32))}
    function f() { a() }
    function g() { sstore(0, 1) } /* Does not invalidate state */
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a: invalidatesState, invalidatesStorage, invalidatesMemory
// f: invalidatesState, invalidatesStorage, invalidatesMemory
// g: invalidatesStorage
