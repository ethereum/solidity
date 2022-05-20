contract C {
    function m(
        function() external returns (uint) a,
        function() external returns (uint) b
    ) internal returns (function() external returns (uint)) {
        return a;
    }

    function s(uint a, uint b) internal returns (uint) {
        return a + b;
    }

    function foo() external returns (uint) {
        return 6;
    }

    function test() public returns (uint) {
        function(uint, uint) internal returns (uint) single_slot_function = s;

        function(
            function() external returns (uint),
            function() external returns (uint)
        ) internal returns (function() external returns (uint)) multi_slot_function = m;

        return multi_slot_function(this.foo, this.foo)() + single_slot_function(5, 1);
    }
}
// ----
// test() -> 12
