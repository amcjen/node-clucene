var clucene = require("./build/default/clucene");

exports.CLucene = clucene;

exports.Store = {
  STORE_YES: 1,
  STORE_NO: 2,
  STORE_COMPRESS: 4
};

exports.Index = {
  INDEX_NO: 16,
  INDEX_TOKENIZED: 32,
  INDEX_UNTOKENIZED: 64,
  INDEX_NONORMS: 128,
};

exports.TermVector = {
  TERMVECTOR_NO: 256,
  TERMVECTOR_YES: 512,
  TERMVECTOR_WITH_POSITIONS: 512 | 1024,
  TERMVECTOR_WITH_OFFSETS: 512 | 2048,
  TERMVECTOR_WITH_POSITIONS_OFFSETS: (512 | 1024) | (512 | 2048) 
};
