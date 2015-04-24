var chai = require('chai');
var assert = chai.assert;
var coder = require('../lib/solidity/coder');

var tests = [
    { type: 'int', value: 1,                expected: '0000000000000000000000000000000000000000000000000000000000000001'},
    { type: 'int', value: 16,               expected: '0000000000000000000000000000000000000000000000000000000000000010'},
    { type: 'int', value: -1,               expected: 'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'},
    { type: 'bytes32', value: 'gavofyork',  expected: '6761766f66796f726b0000000000000000000000000000000000000000000000'},
    { type: 'bytes', value: 'gavofyork',    expected: '0000000000000000000000000000000000000000000000000000000000000009' + 
                                                      '6761766f66796f726b0000000000000000000000000000000000000000000000'}
];

describe('lib/solidity/coder', function () {
    describe('encodeParam', function () {
        tests.forEach(function (test) {
            it('should turn ' + test.value + ' to ' + test.expected, function () {
                assert.equal(coder.encodeParam(test.type, test.value), test.expected);
            });
        });
    });
});

