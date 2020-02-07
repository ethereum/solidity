contract C {
    function f() public pure returns (bytes4) {
        function() external g;
        // Make sure g.selector is not considered pure:
        // If it was considered pure, this would emit a warning "Statement has no effect".
        g.selector;
    }
}
