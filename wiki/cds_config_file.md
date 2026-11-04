# CDS 配置文件

> 下列阐述如无特殊说明，仅对于这个标准而言

## 简述

CDS 配置文件，是一种较为易于程序修改和解析的结构化纯文本配置文件，不支持注释
> 关于扁平化配置文件，请查看[KSV 配置文件](ksv_config_file)

## 规范

CDS 配置文件 采取缩进表示的层级和基于继承的归属关系，为了便于说明和理解，同时基于实际设计方面，你可以将CDS配置文件视为一棵树，故而此，接下来的描述中将会大量使用关于树这种数据结构的术语

- 0: Root  
Root 是整棵树的根，是所有节点的默认父亲和共同祖先，所有没有明确父亲或者不存在父亲的节点，都属于 Root 根节点的儿子，需要注意的是，为了便于计算，我们这里将 Root 节点视为-1层，所以，Root 节点的子节点都属于0层

- 1: 叶子节点  
叶子节点可以存储一个键值对，他的键就是叶子节点的名称或者说是编号，键值对的格式如下  
    - 键  
    键是一个不含有空格和换行符的字符串，比如 `Siyrock`
    - 值  
    值是一个不包含换行符的字符串，比如 `https://vingturbo.pages.dev/`
    - 键值对  
    键与值中间以一个空格分隔，键在前，值在后，比如 `Siyrock https://vingturbo.pages.dev/`

- 2: 目录节点  
目录节点，也就是度不为0的节点，也就是有儿子的节点，目录节点不能存储数据，只能记录节点名称，也就是一个不含有空格和换行符的字符串，比如 `Conati`

- 3: 缩进  
在这里，如果没有特殊说明，所有的 “缩进” 都是指的4个连续的ASCII空格字符，同时可以使用数量进行修饰

- 4: 树的表示结构  
首先，一个节点的表示字符串前面的缩进数量代表着这个节点所属的层数，比如，对于 Root 节点的所有儿子节点，他们的层数都是0，也就是说明，这些节点前面的缩进数量为0，即为顶格书写。而当前层级的每一个节点，都归属于这个层级这个部分的父节点。如果遇到一个目录节点，则代表着，当前层级的下一层级此后的部分的父节点即为这个目录节点，直到文件终止或者被另外的目录节点覆盖

- 5: 结构示例  
``` CDS
githubcloud
    src
        main.cpp program cxx main code
        model
            cli.cpp program cxx cli code
            cli.hpp program cxx cli head
```
可以解析成以下树的结构
``` tree
[Root] -> githubcloud
githubcloud -> src
src -> main.cpp (program cxx main code)
src -> model
model -> cli.cpp (program cxx cli code)
model -> cli.hpp (program cxx cli head)
```