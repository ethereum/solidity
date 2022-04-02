contract c {
    uint[] storageArray;
    function f() public {
        storageArray.length = 3;
    }
}
// ----
// TypeError 7567: (72-91='storageArray.length'): Member "length" is read-only and cannot be used to resize arrays.
