contract C {
    function () transient f;
    function (uint) external transient y;
    function () transient internal fti;
    function () internal transient fit;
    function () internal transient internal fiti;
    function () internal internal transient fiit;
}
// ----
// UnimplementedFeatureError 6715: (17-40): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (46-82): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (88-122): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (128-162): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (168-212): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (218-262): Transient storage is not yet implemented.
