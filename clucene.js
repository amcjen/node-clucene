var clucene = require("./build/default/clucene");

exports.CLucene = clucene;

exports.CLucene.STORE_YES = 1;
exports.CLucene.STORE_NO = 2;
exports.CLucene.STORE_COMPRESS = 4;
exports.CLucene.INDEX_NO = 16;
exports.CLucene.INDEX_TOKENIZED = 32;
exports.CLucene.INDEX_UNTOKENIZED = 64;
exports.CLucene.INDEX_NONORMS = 128;
exports.CLucene.TERMVECTOR_NO = 256;
exports.CLucene.TERMVECTOR_YES = 512;
exports.CLucene.TERMVECTOR_WITH_POSITIONS = 512 | 1024;
exports.CLucene.TERMVECTOR_WITH_OFFSETS = 512 | 2048;
exports.CLucene.TERMVECTOR_WITH_POSITIONS_OFFSETS = (512 | 1024) | (512 | 2048);