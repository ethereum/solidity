contract root { function rootFunction() public {} }
contract inter1 is root { function f() public {} }
contract inter2 is root { function f() public {} }
contract derived is root, inter2, inter1 {
    function g() public { f(); rootFunction(); }
}
// ----
