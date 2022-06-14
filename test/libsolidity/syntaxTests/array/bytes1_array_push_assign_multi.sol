contract C {
    bytes1[] x;
    bytes1[] z;
    function f() public {
        (x.push(), x.push()) = (0, 0);
        (((x.push())), (x.push())) = (0, 0);
        ((x.push(), x.push()), x.push()) = ((0, 0), 0);
        (x.push(), x[0]) = (0, 0);
        bytes1[] storage y = x;
        (x.push(), y.push()) = (0, 0);
        (x.push(), z.push()) = (0, 0);
    }
}
// ----
