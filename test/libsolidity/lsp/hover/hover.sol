// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

/// Documenting another contract here.
contract AnotherContract {}

/// User being documented.
contract User
{
    /// Some enum value.
    enum SomeEnum
    {
        Red,
        Blue
    }

    /// publicVariable being documented.
    SomeEnum public publicVariable;
//  ^ @Cursor1
//  ^^^^^^^^ @Cursor1Range

    // not documented
    mapping(int => User.SomeEnum) someRemapping;
//                      ^ @Cursor2
//                      ^^^^^^^^ @Cursor2Range

    /// Documenting the setContract().
    function setValue(User.SomeEnum _value) public
//                    ^ @Cursor3
//                    ^^^^ @Cursor3Range
    {
        publicVariable = _value;
//      ^ @Cursor4
//      ^^^^^^^^^^^^^^ @Cursor4Range
    }

    function createAnotherContract() public returns (AnotherContract)
    {
        return new AnotherContract();
//                 ^ @Cursor5
//                 ^^^^^^^^^^^^^^^ @Cursor5Range
    }
}
// ----
// -> textDocument/hover {
//     "position": @Cursor1
// }
// <- {
//     "contents": {
//         "kind": "markdown",
//         "value": "```solidity\ntype(enum User.SomeEnum)\n```\n\n"
//     },
//     "range": @Cursor1Range
// }
// -> textDocument/hover {
//     "position": @Cursor2
// }
// <- {
//     "contents": {
//         "kind": "markdown",
//         "value": "```solidity\ntype(enum User.SomeEnum)\n```\n\n"
//     },
//     "range": @Cursor2Range
// }
// -> textDocument/hover {
//     "position": @Cursor3
// }
// <- {
//     "contents": {
//         "kind": "markdown",
//         "value": "```solidity\ntype(contract User)\n```\n\nUser being documented.\n\n"
//     },
//     "range": @Cursor3Range
// }
// -> textDocument/hover {
//     "position": @Cursor4
// }
// <- {
//     "contents": {
//         "kind": "markdown",
//         "value": "```solidity\nenum User.SomeEnum\n```\n\n"
//     },
//     "range": @Cursor4Range
// }
// -> textDocument/hover {
//     "position": @Cursor5
// }
// <- {
//     "contents": {
//         "kind": "markdown",
//         "value": "```solidity\ntype(contract AnotherContract)\n```\n\nDocumenting another contract here.\n\n"
//     },
//     "range": @Cursor5Range
// }
