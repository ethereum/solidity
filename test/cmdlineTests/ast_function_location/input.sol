// // SPDX-License-Identifier: GPL-3.0
// pragma solidity >=0.0;

// // vector<Declaration const*> TypeChecker::cleanOverloadedDeclarations(
// // 	Identifier const& _identifier,
// // 	vector<Declaration const*> const& _candidates
// // )
// // function visit(Identifier _identifier ){
// //     IdentifierAnnotation& annotation = _identifier.annotation();
// //     // pp.prototypeLocation();
// // SecondarySourceLocation ssl;
// // for (Declaration* declaration : annotation.overloadedDeclarations){
// //     if (!declaration->location().isValid())
// //     {
// //         // Try to re-construct function definition
// //         string description;
// //         for (auto const& param: declaration->functionType(true)->parameterTypes())
// //             description += (description.empty() ? "" : ", ") + param->humanReadableName();
// //         description = "function " + _identifier.name() + "(" + description + ")";

// //         ssl.append("Candidate: " + description, declaration->location());
// //     }
// //     else
// //         ssl.append("Candidate:", declaration->location());
// // }

// // }
// function f(uint x) pure returns (uint) { return x ** 2; }
// function f() pure returns (uint) { return 42; }
// contract C {
//      f(1);
//     // f.prototypeLocation();
//         // function (uint) pure returns (uint) immutable i = f;
// }

pragma solidity ^0.8.17;

// contract HelloWorld {
//     string public greet = "Hello World!";
// }
function f(uint x) pure returns (uint) { return x ** 2; }
function f() pure returns (uint) { return 42; }
function checkPattern(int _x, int _y,int _xStep, int _yStep)  returns(int startX, int startY, int endX, int endY)
{ 
   
    return (_x, _x, _x, _x);
}

function checkPattern(int _x, int _y)  returns(bool)
{
    return true;
    
}
function checkStealing(uint _x, uint _y) 
{

    for (uint i = 0; i < 3; i++)
    {
        (int xS, int yS, int xE, int yE) = checkPattern(_x, _y,i, i); // error here
    }
}

contract C {
        // function (uint) pure returns (uint) immutable i = f;
        uint a = 1;
        uint b = 1;
        // f(1);
        function callFuncWithKeyValue() public {
          checkStealing(a,b);
        }
}


