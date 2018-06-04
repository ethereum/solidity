pragma experimental "v0.5.0";
contract C {
    function () internal { }
}
// ----
// TypeError: (47-71): Fallback function must be defined as "external".
