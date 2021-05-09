contract c {
    uint[] data;
    function test() public {
        data.push(7);
        data.pop();
    }
}
// ====
// compileViaYul: also
// ----
// test() ->
// storageEmpty -> 1
