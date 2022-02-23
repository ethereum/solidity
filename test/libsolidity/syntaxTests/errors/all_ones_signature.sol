error test266151307();
contract C {
    error test266151307();
}
// ----
// Warning 2519: (40-62): This declaration shadows an existing declaration.
// SyntaxError 2855: (0-22): The selector 0xffffffff is reserved. Please rename the error to avoid the collision.
// SyntaxError 2855: (40-62): The selector 0xffffffff is reserved. Please rename the error to avoid the collision.
