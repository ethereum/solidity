contract C {
    function f() public pure
    {
        // RLO PDF
        bytes memory m = unicode" ok ‮‬";

        // RLO RLO PDF PDF
        m = unicode" ok ‮‮‬‬";

        // RLO RLO RLO PDF PDF PDF
        m = unicode" ok ‮‮‮‬‬‬";
    }
}
// ----
