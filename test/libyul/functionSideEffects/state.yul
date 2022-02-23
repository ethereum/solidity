{
    function a() { pop(call(100, 0x010, 10, 0x00, 32, 0x0100, 32))}
    function f() { a() }
    function g() { sstore(0, 1) }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: writes other state, writes storage, writes memory
// f: writes other state, writes storage, writes memory
// g: writes storage
