# file comparator

## Motivation

I've always found that existing tools for comparing (especially big) files are not fast enough / or lack ability to report what are they doing / what have they found.
You usually want to do it if you are copying your files and want to be extra sure they are successfully copied.
So I build this file comparator program.

## Build

```shell
make
```

## Usage and Example

```
./main.out path/to/one/big/file path/to/the/copy/of/it
```

It will print something like this

At first:
```
Progress: [                                                  ] 0.00% (0/1091964362)
```

In the middle
```
Progress: [#######                                           ] 14.82% (161873920/1091964362)
```

At the end:
```
Progress: [##################################################] 100.00% (1091964362/1091964362)
Number of differences found: 0
```


