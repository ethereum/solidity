var chai = require('chai');
var utils = require('../lib/utils/utils.js');
var assert = chai.assert;

var tests = [
    { value: 1, expected: '0x1' },
    { value: '1', expected: '0x1' },
    { value: '0x1', expected: '0x1'},
    { value: '0x01', expected: '0x1'},
    { value: 15, expected: '0xf'},
    { value: '15', expected: '0xf'},
    { value: '0xf', expected: '0xf'},
    { value: '0x0f', expected: '0xf'},
    { value: -1, expected: '-0x1'},
    { value: '-1', expected: '-0x1'},
    { value: '-0x1', expected: '-0x1'},
    { value: '-0x01', expected: '-0x1'},
    { value: -15, expected: '-0xf'},
    { value: '-15', expected: '-0xf'},
    { value: '-0xf', expected: '-0xf'},
    { value: '-0x0f', expected: '-0xf'},
    { value: '0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff', expected: '0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'},
    { value: '0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd', expected: '0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd'},
    { value: '-0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff', expected: '-0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'},
    { value: '-0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd', expected: '-0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd'},
    { value: 0, expected: '0x0'},
    { value: '0', expected: '0x0'},
    { value: '0x0', expected: '0x0'},
    { value: -0, expected: '0x0'},
    { value: '-0', expected: '0x0'},
    { value: '-0x0', expected: '0x0'}
];

describe('lib/utils/utils', function () {
    describe('fromDecimal', function () {
        tests.forEach(function (test) {
            it('should turn ' + test.value + ' to ' + test.expected, function () {
                assert.equal(utils.fromDecimal(test.value), test.expected);
            });
        });
    });
});

