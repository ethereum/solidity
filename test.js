var chai = require('chai');
var natspec = require('../natspec.js');
var assert = chai.assert;

describe('natspec', function () {
    it('should evaluate simple expression', function () {
        // given
        var expression = "`x = 1` + `y = 2` will be equal `x + y`";

        // when
        var result = natspec.evaluateExpression(expression);
        var result2 = natspec.evaluateExpressionSafe(expression);

        // then
        assert.equal(result, "1 + 2 will be equal 3");
        assert.equal(result2, "1 + 2 will be equal 3");
    });

    it('should evalute expression using input params', function () {
        //given
        var expression = "Will multiply `a` by 7 and return `a * 7`.";
        var method = 'multiply';
        var abi = [{
            "name": "multiply",
            "constant": false,
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }],
            "outputs": [{
                "name": "d",
                "type": "uint256"
            }]
        }];

        var transaction = {
            "jsonrpc": "2.0",
            "method": "eth_call",
            "params": [{
                "to": "0x8521742d3f456bd237e312d6e30724960f72517a",
                "data": "0xc6888fa1000000000000000000000000000000000000000000000000000000000000007a"
            }],
            "id": 6
        };
        
        var call = {
            method: method,
            abi: abi,
            transaction: transaction
        };

        // when
        var result = natspec.evaluateExpression(expression, call);
        var result2 = natspec.evaluateExpressionSafe(expression, call);
        
        // then
        assert.equal(result, "Will multiply 122 by 7 and return 854.");
        assert.equal(result2, "Will multiply 122 by 7 and return 854.");
    });
    
    it('should evalute expression using input params', function () {
        //given
        var expression = "Will multiply `a` by 7 and return `a * 7`.";
        var method = 'multiply';
        var abi = [{
            "name": "multiply",
            "constant": false,
            "type": "function",
            "inputs": [{
                "name": "b",
                "type": "uint256"
            }],
            "outputs": [{
                "name": "d",
                "type": "uint256"
            }]
        }];

        var transaction = {
            "jsonrpc": "2.0",
            "method": "eth_call",
            "params": [{
                "to": "0x8521742d3f456bd237e312d6e30724960f72517a",
                "data": "0xc6888fa1000000000000000000000000000000000000000000000000000000000000007a"
            }],
            "id": 6
        };
        
        var call = {
            method: method,
            abi: abi,
            transaction: transaction
        };

        // when
        var exceptionThrow = function () { natspec.evaluateExpression(expression, call);}
        var result = natspec.evaluateExpressionSafe(expression, call); 
        
        // then
        assert.equal(result, "Natspec evaluation failed, wrong input params");
        assert.throws(exceptionThrow, "Natspec evaluation failed, wrong input params");
    });

    it('should evalute expression using input params', function () {
        //given
        var expression = "Will multiply `a` by 7 and return `a * 7`.";
        var method = 'multiply2';
        var abi = [{
            "name": "multiply",
            "constant": false,
            "type": "function",
            "inputs": [{
                "name": "a",
                "type": "uint256"
            }],
            "outputs": [{
                "name": "d",
                "type": "uint256"
            }]
        }];

        var transaction = {
            "jsonrpc": "2.0",
            "method": "eth_call",
            "params": [{
                "to": "0x8521742d3f456bd237e312d6e30724960f72517a",
                "data": "0xc6888fa1000000000000000000000000000000000000000000000000000000000000007a"
            }],
            "id": 6
        };
        
        var call = {
            method: method,
            abi: abi,
            transaction: transaction
        };

        // when
        var exceptionThrow = function () { natspec.evaluateExpression(expression, call);}
        var result = natspec.evaluateExpressionSafe(expression, call); 
        
        // then
        assert.equal(result, "Natspec evaluation failed, method does not exist");
        assert.throws(exceptionThrow, "Natspec evaluation failed, method does not exist");
    });
});


