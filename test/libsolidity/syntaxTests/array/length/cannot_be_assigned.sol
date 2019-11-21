contract c {
    uint[] storageArray;
    function f() public {
        storageArray.length = 3;
    }
}
// ----
// TypeError: (72-91): Member "length" is read-only and cannot be used to resize arrays.
