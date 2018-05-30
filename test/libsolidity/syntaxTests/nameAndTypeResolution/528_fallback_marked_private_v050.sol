pragma experimental "v0.5.0";
contract C {
    function () private { }
}
// ----
// TypeError: (47-70): Fallback function must be defined as "external".
