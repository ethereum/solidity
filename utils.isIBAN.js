var chai = require('chai');
var utils = require('../lib/utils/utils.js');
var assert = chai.assert;

var tests = [
    { obj: function () {}, is: false},
    { obj: new Function(), is: false},
    { obj: 'function', is: false},
    { obj: {}, is: false},
    { obj: '[]', is: false},
    { obj: '[1, 2]', is: false},
    { obj: '{}', is: false},
    { obj: '{"a": 123, "b" :3,}', is: false},
    { obj: '{"c" : 2}', is: false},
    { obj: 'XE81ETHXREGGAVOFYORK', is: true},
    { obj: 'XE81ETCXREGGAVOFYORK', is: false},
    { obj: 'XE81ETHXREGGAVOFYORKD', is: false},
    { obj: 'XE81ETHXREGGaVOFYORK', is: false},
    { obj: 'XE7338O073KYGTWWZN0F2WZ0R8PX5ZPPZS', is: true},
    { obj: 'XD7338O073KYGTWWZN0F2WZ0R8PX5ZPPZS', is: false}
];

describe('lib/utils/utils', function () {
    describe('isIBAN', function () {
        tests.forEach(function (test) {
            it('shoud test if value ' + test.obj + ' is iban: ' + test.is, function () {
                assert.equal(utils.isIBAN(test.obj), test.is);
            });
        });   
    });
});

