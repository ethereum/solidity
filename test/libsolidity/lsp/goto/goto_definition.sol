// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import "./lib.sol";
//      ^ @importDirective

interface I
{
    function f(uint x) external returns (uint);
    //       ^ @functionF
}

contract IA is I
    //   ^^ @IASymbol
{
    function f(uint x) public pure override returns (uint) { return x + 1; }
}

contract IB is I
{
    function f(uint x) public pure override returns (uint) { return x + 2; }
}

library IntLib
{
    function add(int self, int b) public pure returns (int) { return self + b; }
    //       ^^^ @IntLibAdd
}

contract C
{
    I obj;
    function virtual_inheritance() public payable
    {
        obj = new IA();
        //        ^ @usingIASymbol
        obj.f(1); // goto-definition should jump to definition of interface.
    //      ^ @virtualFunctionLookup
    }

    using IntLib for *;
    function using_for(int i) pure public
    {
        i.add(5);
 //       ^ @usingIntAdd
        14.add(4);
    }

    function useLib(uint n) public payable returns (uint)
    {
        return Lib.add(n, 1);
        //     ^ @LibSymbol
        //         ^ @LibAddSymbol
    }

    function enums(Color c) public pure returns (Color d)
    //             ^ @ColorSymbolInParameter
    {
        Color e = Color.Red;
        //    ^ @eVariableDeclaration
        //              ^ @RedEnumMemberAccess
        if (c == e)
        //       ^ @eVariableAccess
            d = Color.Green;
        else
            d = c;
    }

    type Price is uint128;
    //   ^^^^^ @PriceDeclaration
    function udlTest() public pure returns (uint128)
    {
        Price p = Price.wrap(128);
    //  ^ @PriceSymbol
    //            ^ @PriceInWrap
        return Price.unwrap(p);
    }

    function structCtorTest(uint8 v) public pure returns (uint8 result)
    {
        RGBColor memory c = RGBColor(v, 2 * v, 3 * v);
        //                       ^ @RGBColorCursor
        result = c.red;
        int a;
//      ^^^^^ @unusedLocalVar
    }
}
// ----
// goto_definition: @unusedLocalVar 2072
// lib: @diagnostics 2072
// -> textDocument/definition {
//     "position": @importDirective
// }
// <- [
//     {
//         "range": {
//             "end": {
//                 "character": 0,
//                 "line": 0
//             },
//             "start": {
//                 "character": 0,
//                 "line": 0
//             }
//         },
//         "uri": "lib.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @usingIASymbol
// }
// <- [
//     {
//         "range": @IASymbol,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @virtualFunctionLookup
// }
// <- [
//     {
//         "range": @functionF,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @usingIntAdd
// }
// <- [
//     {
//         "range": @IntLibAdd,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @LibSymbol
// }
// <- [
//     {
//         "range": @LibLibrary,
//         "uri": "lib.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @LibAddSymbol
// }
// <- [
//     {
//         "range": @addSymbol,
//         "uri": "lib.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @ColorSymbolInParameter
// }
// <- [
//     {
//         "range": @ColorEnum,
//         "uri": "lib.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @RedEnumMemberAccess
// }
// <- [
//     {
//         "range": @EnumMemberRed,
//         "uri": "lib.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @eVariableAccess
// }
// <- [
//     {
//         "range": @eVariableDeclaration,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @PriceSymbol
// }
// <- [
//     {
//         "range": @PriceDeclaration,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @PriceInWrap
// }
// <- [
//     {
//         "range": @PriceDeclaration,
//         "uri": "goto_definition.sol"
//     }
// ]
// -> textDocument/definition {
//     "position": @RGBColorCursor
// }
// <- [
//     {
//         "range": @RGBColorStruct,
//         "uri": "lib.sol"
//     }
// ]
