// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

enum Color {
//   ^ @EnumDef
//   ^^^^^ @ColorType0
    Red,
//  ^ @RedDef
//  ^^^ @Red1
    Green
}

contract MyContract
{
    Color lastColor = Color.Red;
//            ^ @lastCursorDef
//        ^^^^^^^^^ @lastCursor1
//                    ^^^^^^^^^ @Red2
//  ^^^^^ @ColorType1
//                    ^^^^^ @ColorType2

    function sum(Color a) public view returns (Color)
//               ^^^^^ @ColorType3
//                                             ^^^^^ @ColorType4
    {
        Color result = Color(a);
//      ^^^^^ @ColorType5
//                     ^^^^^ @ColorType6
        if (a != lastColor)
//               ^^^^^^^^^ @lastCursor2
            result = Color.Green;
//                   ^^^^^ @ColorType7
        return result;
    }

    function f() public pure returns (uint)
    {
        uint result = 42;
        return result;
    }
}

// ----
// -> textDocument/documentHighlight {
//     "position": @EnumDef
// }
// <- [
//     { "kind": 2, "range": @ColorType0 },
//     { "kind": 2, "range": @ColorType1 },
//     { "kind": 2, "range": @ColorType2 },
//     { "kind": 2, "range": @ColorType3 },
//     { "kind": 2, "range": @ColorType4 },
//     { "kind": 2, "range": @ColorType5 },
//     { "kind": 2, "range": @ColorType6 },
//     { "kind": 2, "range": @ColorType7 }
// ]
// -> textDocument/documentHighlight {
//     "position": @RedDef
// }
// <- [
//     { "kind": 2, "range": @Red1 },
//     { "kind": 2, "range": @Red2 }
// ]
// -> textDocument/documentHighlight {
//     "position": @lastCursorDef
// }
// <- [
//     { "kind": 3, "range": @lastCursor1 },
//     { "kind": 2, "range": @lastCursor2 }
// ]
