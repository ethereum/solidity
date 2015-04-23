var chai = require('chai');
var assert = chai.assert;
var coder = require('../lib/solidity/coder');

var tests = [
    { type: 'int', value: 1,    expected: '0000000000000000000000000000000000000000000000000000000000000001'},
    { type: 'int', value: 16,   expected: '0000000000000000000000000000000000000000000000000000000000000010'},
    { type: 'int', value: -1,   expected: 'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'}
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

