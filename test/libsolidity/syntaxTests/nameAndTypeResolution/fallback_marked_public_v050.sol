pragma experimental "v0.5.0";
contract C {
    function () public { }
}
// ----
// TypeError: (47-69): Fallback function must be defined as "external".
