contract c {
    uint[] a;
    function f() returns (uint, uint) {
        a = [1,2,3];
        return (a[3], [2,3,4][0]);
    }
}
// ----
// Warning: (31-128): No visibility specified. Defaulting to "public". 
