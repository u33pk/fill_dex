# 抽取壳填充

针对 [u33pk](https://github.com/u33pk/u33pk) 脱掉抽取壳后的code item进行回填

## 用法

```bash
make
./dex path/classes.dex path/item_size.item

#或者
python3 util/qc.py  # 该工具内的路径和文件名自行修改
```
## 使用的库

1. [base64](https://github.com/zhicheng/base64)
2. [DexFile.h](http://androidxref.com/9.0.0_r3/xref/dalvik/libdex/DexFile.h)

## 更新日志

| 日期 | 摘要|
|-------|-------|
| 23.6.30 | 完成u33pk脱壳item文件回填的基础功能 |
| 24.4.20 | 修复回填的0偏移和存在nop的错误, 增加了一个item去重的工具 |