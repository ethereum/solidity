var chai = require('chai');
var assert = require('assert');
var abi = require('../lib/solidity/abi');

describe('lib/solidity/abi', function () {
    describe('formatConstructorParams', function () {
        it('should format uint256 properly', function () {
            // given
            var description =  [{
                "name": "test",
                "type": "constructor",
                "inputs": [{
                    "name": "a",
                    "type": "uint256"
                }
                ]
            }];

            // when 
            var bytes = abi.formatConstructorParams(description, [2]);

            // then
            assert.equal(bytes, '0000000000000000000000000000000000000000000000000000000000000002');
        });
        
        it('should not find matching constructor', function () {
            // given
            var description =  [{
                "name": "test",
                "type": "constructor",
                "inputs": [{
                    "name": "a",
                    "type": "uint256"
                }
                ]
            }];

            // when 
            var bytes = abi.formatConstructorParams(description, []);

            // then
            assert.equal(bytes, '');
        });

        it('should not find matching constructor2', function () {
            // given
            var description =  [{
                "name": "test",
                "type": "constructor",
                "inputs": [{
                    "name": "a",
                    "type": "uint256"
                }
                ]
            }];

            // when 
            var bytes = abi.formatConstructorParams(description, [1,2]);

            // then
            assert.equal(bytes, '');
        });
        
        it('should not find matching constructor3', function () {
            // given
            var description =  [{
                "name": "test",
                "type": "function",
                "inputs": [{
                    "name": "a",
                    "type": "uint256"
                }
                ]
            }];

            // when 
            var bytes = abi.formatConstructorParams(description, [2]);

            // then
            assert.equal(bytes, '');
        });

        it('should find matching constructor with multiple args', function () {
            // given
            var description =  [{
                "name": "test",
                "type": "constructor",
                "inputs": [{
                    "name": "a",
                    "type": "uint256"
                }, {
                    "name": "b",
                    "type": "uint256"
                }]
            }];

            // when 
            var bytes = abi.formatConstructorParams(description, ['1', '5']);

            // then
            assert.equal(bytes, '00000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000005');
        });
    });
});


