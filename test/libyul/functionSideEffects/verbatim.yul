{
    function a() -> x { x := verbatim_0i_1o(hex"6007") }
    function b() { let t := a() }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: can loop, writes other state, writes storage, writes memory
// b: can loop, writes other state, writes storage, writes memory
