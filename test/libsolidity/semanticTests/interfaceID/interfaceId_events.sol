interface HelloWorld {
    function hello() external pure;
    function world(int) external pure;
}

interface HelloWorldWithEvent {
    event Event();
    function hello() external pure;
    function world(int) external pure;
}

contract Test {
    bytes4 public hello_world = type(HelloWorld).interfaceId;
    bytes4 public hello_world_with_event = type(HelloWorldWithEvent).interfaceId;
}

// ====
// compileToEwasm: also
// ----
// hello_world() -> left(0xc6be8b58)
// hello_world_with_event() -> left(0xc6be8b58)
