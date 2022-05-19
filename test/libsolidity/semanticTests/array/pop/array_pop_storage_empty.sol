contract c {
    uint[] data;
    function test() public {
        data.push(7);
        data.pop();
    }
}
// ----
// test() ->
// storageEmpty -> 1
