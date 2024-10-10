contract C {
    function () transient f;
    function (uint) external transient y;
    function () transient internal fti;
    function () internal transient fit;
    function () internal transient internal fiti;
    function () internal internal transient fiit;
}
// ====
// EVMVersion: >=cancun
// ----
