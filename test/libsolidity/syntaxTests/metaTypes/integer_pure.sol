contract test {

    function viewAssignment() public view {
        int min = type(int).min;
        min;
    }

    function assignment() public {
        int max = type(int).max;
        max;
    }

}
// ----
// Warning 2018: (21-112): Function state mutability can be restricted to pure
// Warning 2018: (118-200): Function state mutability can be restricted to pure
