var chai = require('chai');
var assert = chai.assert;
var coder = require('../lib/solidity/coder');

var tests = [
    { type: 'int',      value: '0000000000000000000000000000000000000000000000000000000000000001', expected: 1},
    { type: 'int',      value: '0000000000000000000000000000000000000000000000000000000000000010', expected: 16},
    { type: 'int',      value: 'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff', expected: -1},
    { type: 'bytes32',  value: '6761766f66796f726b0000000000000000000000000000000000000000000000', expected: 'gavofyork'},
    { type: 'bytes',    value: '0000000000000000000000000000000000000000000000000000000000000009' + 
                               '6761766f66796f726b0000000000000000000000000000000000000000000000', expected: 'gavofyork'}
];

describe('lib/solidity/coder', function () {
    describe('decodeParam', function () {
        tests.forEach(function (test) {
            it('should turn ' + test.value + ' to ' + test.expected, function () {
                assert.equal(coder.decodeParam(test.type, test.value), test.expected);
            });
        });
    });
});

