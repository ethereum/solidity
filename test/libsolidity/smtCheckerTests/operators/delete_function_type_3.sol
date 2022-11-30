contract C {
    function()[] arr;
    function() f;

    constructor() {
        arr.push();
        arr.push();
        arr[1] = f;
        delete arr[1];
        assert(arr[1] == f); // should fail for now because function pointer comparisons are not supported
    }
}
// ----
// Warning 8364: (82-90): Assertion checker does not yet implement type function (function ()[] storage pointer) returns (function ())
// Warning 8364: (102-110): Assertion checker does not yet implement type function (function ()[] storage pointer) returns (function ())
// Warning 7229: (172-183): Assertion checker does not yet implement the type function () for comparisons
// Warning 6368: (149-155): CHC: Out of bounds access happens here.
// Warning 6368: (172-178): CHC: Out of bounds access happens here.
// Warning 6328: (165-184): CHC: Assertion violation happens here.
