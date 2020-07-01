{
    function a() { a() }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: movable apart from effects, can loop
