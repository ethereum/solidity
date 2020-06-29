{
    function a() {}
    function f() { mstore(0, 1) }
    function g() { sstore(0, 1) }
    function h() { let x := msize() }
    function i() { let z := mload(0) }
}
// ----
// : movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// a: movable, movableIfStateInvariant, movableIfStorageInvariant, movableIfMemoryInvariant, sideEffectFree, sideEffectFreeIfNoMSize
// f: invalidatesMemory
// g: invalidatesStorage
// h: sideEffectFree, sideEffectFreeIfNoMSize
// i: movableIfMemoryInvariant, sideEffectFreeIfNoMSize
