var chai = require('chai');
var assert = chai.assert;
var coder = require('../lib/solidity/coder');
var BigNumber = require('bignumber.js');
var bn = BigNumber;


describe('lib/solidity/coder', function () {
    describe('decodeParam', function () {
        var test = function (t) {
            it('should turn ' + t.value + ' to ' + t.expected, function () {
                assert.deepEqual(coder.decodeParam(t.type, t.value), t.expected);
            });
        };


        test({ type: 'int', expected: new bn(1),            value: '0000000000000000000000000000000000000000000000000000000000000001'});
        test({ type: 'int', expected: new bn(16),           value: '0000000000000000000000000000000000000000000000000000000000000010'});
        test({ type: 'int', expected: new bn(-1),           value: 'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'});
        test({ type: 'int256', expected: new bn(1),         value: '0000000000000000000000000000000000000000000000000000000000000001'});
        test({ type: 'int256', expected: new bn(16),        value: '0000000000000000000000000000000000000000000000000000000000000010'});
        test({ type: 'int256', expected: new bn(-1),        value: 'ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff'});
        test({ type: 'bytes32', expected: 'gavofyork',      value: '6761766f66796f726b0000000000000000000000000000000000000000000000'});
        test({ type: 'bytes', expected: 'gavofyork',        value: '0000000000000000000000000000000000000000000000000000000000000009' + 
                                                                   '6761766f66796f726b0000000000000000000000000000000000000000000000'});
        test({ type: 'int[]', expected: [new bn(3)],        value: '0000000000000000000000000000000000000000000000000000000000000001' +
                                                                   '0000000000000000000000000000000000000000000000000000000000000003'});
        test({ type: 'int256[]', expected: [new bn(3)],     value: '0000000000000000000000000000000000000000000000000000000000000001' +
                                                                   '0000000000000000000000000000000000000000000000000000000000000003'});
        test({ type: 'int[]', expected: [new bn(1), new bn(2), new bn(3)],
                                                            value: '0000000000000000000000000000000000000000000000000000000000000003' +
                                                                   '0000000000000000000000000000000000000000000000000000000000000001' +
                                                                   '0000000000000000000000000000000000000000000000000000000000000002' +
                                                                   '0000000000000000000000000000000000000000000000000000000000000003'});
    });
});

describe('lib/solidity/coder', function () {
    describe('decodeParams', function () {
        var test = function (t) {
            it('should turn ' + t.values + ' to ' + t.expected, function () {
                assert.deepEqual(coder.decodeParams(t.types, t.values), t.expected);
            });
        };


        test({ types: ['int'], expected: [new bn(1)],       values: '0000000000000000000000000000000000000000000000000000000000000001'});
        test({ types: ['bytes32', 'int'], expected: ['gavofyork', new bn(5)],
                                                            values: '6761766f66796f726b0000000000000000000000000000000000000000000000' + 
                                                                    '0000000000000000000000000000000000000000000000000000000000000005'});
        test({ types: ['int', 'bytes32'], expected: [new bn(5), 'gavofyork'],
                                                            values: '0000000000000000000000000000000000000000000000000000000000000005' + 
                                                                    '6761766f66796f726b0000000000000000000000000000000000000000000000'});
        test({ types: ['int', 'bytes', 'int', 'int', 'int', 'int[]'], expected: [new bn(1), 'gavofyork', new bn(2), new bn(3), new bn(4), 
                                                                                 [new bn(5), new bn(6), new bn(7)]],
                                                            values: '0000000000000000000000000000000000000000000000000000000000000009' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000003' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000001' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000002' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000003' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000004' +
                                                                    '6761766f66796f726b0000000000000000000000000000000000000000000000' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000005' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000006' +
                                                                    '0000000000000000000000000000000000000000000000000000000000000007'});
    });
});

