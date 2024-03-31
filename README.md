# gob_stdmap

[日本語](#概要)

## Overview
Although std::map is convenient, it tends to use a lot of memory due to its internal implementation.  
Therefore, std::map is implemented using std::vector to reduce memory usage.  
The class function is compatible with std::map, so it works simply by replacing the std::map used in conventional code.  
Although it has the appearance of an Arduino Library, it can also be used on other systems.

## How to install
Install in an appropriate way depending on your environment.
* git clone or download zip, and extract into place
* platformio.ini
```ini
lib_deps = https://github.com/GOB52/gob_stdmap.git
```

## How to use

```cpp
#include <iostream>
#include <gob_stdmap.hpp>

int main()
{
  // Equivalent behaviour with functions in std::map
  goblib::stdmap<char, int> m;

  m.insert(std::make_pair('c', 30));
  m.insert(std::make_pair('a', 10));
  m.insert(std::make_pair('b', 20));

  int value = m.at('a');
  std::cout << value << std::endl;

  return 0;
}
```

## Differences with std::map

### Behaviour and type

||std::map| gobllib::stdmap|
|---|---|---|
|**class::value_type**| std::pair<const Key, T> | std::pair<Key, T> |
|**template class Allocator** | std::pair<const Key, T> | std::pair<Key, T> |
|**Iterator**| bidirectional | random access |
|**Iterator lifetime**| **Not broken** by the execution of the insert or erase | **Broken** by the execution of the insert or erase |

### Performance
- Measurements on M5Stack Core2
- Using std::map<int,int> , goblib::stdmap<int,int>
- Unit of the Memory usgae is byte
- Unit of the elapsed time is us

The memory usage was about 1/5 of std::map.  
When the number of elements is small, goblib::stdmap tends to be better, except for find.

- 10 elements

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|404|96| 1 : 0.24|
|**insert**|188|15| 1 : 0.08 |
|**find**|4|15| 1: 3.75 |
|**iteration**|6|2| 1 : 0.33 |
|**erase**|78|12|1 : 0.15 |

- 100 elements

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|4000|816| 1 : 0.20 |
|**insert**|1466|198| 1 : 0.14 |
|**find**|45|125| 1 : 2.78 |
|**iteration**|22|5| 1 : 0.23 |
|**erase**|580|248| 1 : 0.43 |

- 1000 elements

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|40012|8016| 1 : 0.20 |
|**insert**|14770|13055| 1 : 0.88 |
|**find**|625|1743| 1 : 2.79 |
|**iteration**|204|51| 1 : 0.25 |
|**erase**|5969|14404| 1 : 2.41 |


- 10000 elements

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|400000|80016| 1 : 0.20 |
|**insert**|309348|6874837| 1 : 22:22 |
|**find**|26592|51560| 1 : 1.94 |
|**iteration**|12706|4473 |1 : 0.35 |
|**erase**|79850|6991864 |1 : 87.56 |

See also [benchmark.cpp](test/embedded/test_benchmark/benchmark.cpp)

---

## 概要

std::map は便利ですが、内部実装の都合上大量のメモリを使う傾向にあります。  
そこで std::vector による実装によってメモリ使用量を抑えたのが、この stdmap です。  
クラス関数は std::map と互換ですので、従来のコードで使用している std::map を置き換えるだけで動作します。  
Arduino Library の体裁をとっていますが、他のシステムでも使用可能です。

<!-- AssocVector という同様に std::vector による実装がされている物がありますが、 -->



## 導入
環境によって適切な方法でインストールしてください
* git clone や Zip ダウンロードからの展開
* platformio.ini
```ini
lib_deps = https://github.com/GOB52/gob_stdmap.git
```

## 使い方

```cpp
#include <iostream>
#include <gob_stdmap.hpp>

int main()
{
  // Equivalent behaviour with functions in std::map
  goblib::stdmap<char, int> m;

  m.insert(std::make_pair('c', 30));
  m.insert(std::make_pair('a', 10));
  m.insert(std::make_pair('b', 20));

  int value = m.at('a');
  std::cout << value << std::endl;

  return 0;
}
```

## std::map との相違点

### 挙動や型

||std::map| gobllib::stdmap|
|---|---|---|
|**class::value_type**| std::pair<const Key, T> | std::pair<Key, T> |
|**template class Allocator** | std::pair<const Key, T> | std::pair<Key, T> |
|**Iterator**| bidirectional | random access |
|**Iteratorの寿命 **| 挿入や削除で**壊れない**| 挿入や削除で**壊れる**|

### 性能

- M5Stack Core2 での計測
- std::map<int,int> , goblib::stdmap<int,int> を使用
- メモリ使用量の単位はバイト
- 経過時間の単位は us

メモリ使用量は std::map の 1/5 程度となりました。  
要素数が少ない場合は、探索以外は goblib::stdmap の方が良い傾向にあります。


- 要素数 10

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|404|96| 1 : 0.24|
|**insert**|188|15| 1 : 0.08 |
|**find**|4|15| 1: 3.75 |
|**iteration**|6|2| 1 : 0.33 |
|**erase**|78|12|1 : 0.15 |

- 要素数 100

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|4000|816| 1 : 0.20 |
|**insert**|1466|198| 1 : 0.14 |
|**find**|45|125| 1 : 2.78 |
|**iteration**|22|5| 1 : 0.23 |
|**erase**|580|248| 1 : 0.43 |

- 要素数 1000

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|40012|8016| 1 : 0.20 |
|**insert**|14770|13055| 1 : 0.88 |
|**find**|625|1743| 1 : 2.79 |
|**iteration**|204|51| 1 : 0.25 |
|**erase**|5969|14404| 1 : 2.41 |


- 要素数 10000

||std::map|goblib::stdmap|Ratio|
|---|---|---|---|
|**Memory usage**|400000|80016| 1 : 0.20 |
|**insert**|309348|6874837| 1 : 22:22 |
|**find**|26592|51560| 1 : 1.94 |
|**iteration**|12706|4473 |1 : 0.35 |
|**erase**|79850|6991864 |1 : 87.56 |

ベンチマークについては [benchmark.cpp](test/embedded/test_benchmark/benchmark.cpp) を参照。

